#if defined(ARENA_SERVER)
#include "server.h"
#include <stdio.h>
#include <assert.h>
#include "messageIdentifiers.h"
#include <bx/timer.h>

// Help: http://enet.bespin.org/Tutorial.html#Connecting
Server::Server()
{
	m_clientAmount = 0;
	m_world.gravity = -9.81f;
	m_world.limits.x = 1000;
	m_world.limits.y = 1000;
	m_b2DWorld = new b2World(b2Vec2(0.0f, 9.81f));
	
	b2Vec2 platform3[6];
	platform3[0].Set(0,0);
	platform3[1].Set(0,500);
	platform3[2].Set(250,650);
	platform3[3].Set(750,650);
	platform3[4].Set(1000,500);
	platform3[5].Set(1000,0);

	Platform platform;


	platform.m_shape.CreateChain(platform3, 6);
	platform.m_bodydef.type = b2_staticBody;
	platform.m_bodydef.position.Set(0, 0);
	platform.m_body = m_b2DWorld->CreateBody(&platform.m_bodydef);
	platform.m_fixtureDef.shape = &platform.m_shape;
	platform.m_fixtureDef.density = 1.0f;
	platform.m_fixtureDef.friction = 0.3f;
	platform.m_body->CreateFixture(&platform.m_fixtureDef);

	m_platformVector.push_back(platform);

}

Server::~Server()
{
	// Free up resources used by enet.
	enet_host_destroy(m_server);
	enet_deinitialize();
	
	enet_host_destroy(m_server);
	
}

void Server::start(unsigned port, unsigned playerAmount)
{
	initializeENet();
	m_server = createENetServer(0, port, playerAmount);
	
	for (int i = 0; i< playerAmount; i++)
	{
		Gladiator glad;
		glad.m_id = i;
		glad.m_position.x = 100*i+100;
		glad.m_position.y = 65;
		glad.m_velocity.x = 0;
		glad.m_velocity.y = 0;
		glad.m_rotation = 17;
		
		b2BodyDef bodydef;
		bodydef.type = b2_dynamicBody;
		bodydef.position.Set(50.0f*i, 65.0f);
		glad.m_body = m_b2DWorld->CreateBody(&bodydef);

		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(1.0f, 1.0f);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.3f;

		b2MassData data;
		data.mass = 1;
		data.center = b2Vec2(20,20);
		

		glad.m_body->SetMassData(&data);

		glad.m_body->CreateFixture(&fixtureDef);
		m_gladiatorVector.push_back(glad);
	}
	
	while (m_clientAmount < playerAmount)
		checkEvent();
	// reserve space for update message.
	m_updateSize = sizeof(MessageIdentifiers) + sizeof(float) * 5 * m_gladiatorVector.size();;
	m_updateMemory =(unsigned char*)malloc(m_updateSize);
	
	for(unsigned i = 0; i < m_clientAmount; i++)
		{
			size_t size;
			unsigned char* data = createGameSetupPacket(m_clientAmount, i, size);
			sendPacket(data, size, i);
		}

	int64_t s_last_time = bx::getHPCounter();
	float updatePhysics = 0;
	float updateNetwork = 0;

	float timeStep = 1.0f/60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;
	while (true)
	{
		int64_t currentTime = bx::getHPCounter();
		const int64_t time = currentTime - s_last_time;
		s_last_time = currentTime;

		const double frequency = (double)bx::getHPFrequency();

		// seconds
		float lastDeltaTime = float(time * (1.0f / frequency));
		updatePhysics += lastDeltaTime;
		updateNetwork += lastDeltaTime;
		if (updatePhysics > timeStep)
		{
			
			updatePhysics = 0;
			m_b2DWorld->Step(timeStep, velocityIterations, positionIterations);
			for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
			{
				m_gladiatorVector[i].m_body->ApplyForce(b2Vec2(m_clientArray[i].input.x*100, m_clientArray[i].input.y*100),
									 m_gladiatorVector[i].m_body->GetWorldCenter(), 1);
				m_clientArray[i].input.x = 0;
				m_clientArray[i].input.y = 0;
				b2Vec2 position = m_gladiatorVector[i].m_body->GetPosition();
				m_gladiatorVector[i].m_position.x = position.x;
				m_gladiatorVector[i].m_position.y = position.y;
				printf("Physics: %f, %f\n", position.x, position.y);
			}
		}
		// Pushes new messages from players to their queues
		checkEvent();

		/*// Handle messages
		for (unsigned playerIndex= 0; playerIndex < playerAmount; playerIndex++)
		{
			while (m_clientArray[playerIndex].messageQueue.size() != 0)
			{
				unsigned char* data;
				data = m_clientArray[playerIndex].messageQueue.front();
				// handle data here
				receiveMovePacket(data);
				m_clientArray[playerIndex].messageQueue.pop();

			}
		}*/
		// PHYSICS AND STUFF AND THEN SEND
		if(updateNetwork > 0.1)
		{
			size_t size;
			printf("updating network \n");
			unsigned char *data = createGameUpdatePacket(m_gladiatorVector, size);
			broadcastPacket(data, size);
			updateNetwork = 0;
		}
	}
}

void Server::initializeENet()
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occured while intializing ENET. \n");	
		abort();
	}
}

ENetHost* Server::createENetServer(unsigned address, unsigned port, unsigned playerAmount)
{
	ENetAddress eAddress;
	ENetHost* eServer;

	if (address == 0)
		eAddress.host = ENET_HOST_ANY;
	else
		eAddress.host = address;
	eAddress.port = port;

	eServer = enet_host_create(&eAddress /* the address to bind the server host to */,
		playerAmount      /* allow up to playerAmount clients and/or outgoing connections */,
		2      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);


	//for (int i = 0; i < playerAmount; i++)
	//{
	//	ENetEvent EEvent;
	//	m_clientArray[playerAmount] = EEvent;
	//}

	if (eServer == NULL)
	{
		fprintf(stderr, "An error occured while trying to create ENet server host \n");
		abort();
	}
	return eServer;
}

void Server::checkEvent()
{
	// Does event have to persist after one loop? peer->data is saved
	ENetEvent EEvent;
	enet_host_service(m_server, &EEvent, 0);

	switch (EEvent.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		printf("A new client connected from %x:%u.\n",
			EEvent.peer->address.host,
			EEvent.peer->address.port);
		EEvent.peer->data = (void*)m_clientAmount;
		m_clientArray[m_clientAmount].peer = EEvent.peer;
		m_clientAmount++;
		break;
	
	case ENET_EVENT_TYPE_RECEIVE:
	{
		printf("A packet of length %u containing %s was received from %d on channel %u.\n",
			EEvent.packet->dataLength,
			EEvent.packet->data,
			EEvent.peer->data,
			EEvent.channelID);
		unsigned id = unsigned(EEvent.peer->data);
		printf("%u\n", id);
		// save data at this point and then destory packet.
		// Data is not saved atm.
		//m_clientArray[id].messageQueue.push(EEvent.packet->data);
		receiveMovePacket(EEvent.packet->data, id);

		enet_packet_destroy(EEvent.packet);
		break;
	}
	case ENET_EVENT_TYPE_DISCONNECT:
		printf("%s disconnected.\n", EEvent.peer->data);
		EEvent.peer->data = NULL;
	default:
		break;
	}

}

// Should reliable and unreliable data use different channels?
void Server::sendPacket(unsigned char* packet, unsigned size, unsigned clientIndex)
{

	ENetPacket *ePacket = enet_packet_create(packet,
											size, 
											ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_clientArray[clientIndex].peer, 0, ePacket);
	// When all packets are ready to send use
	enet_host_flush(m_server);
}

void Server::broadcastPacket(unsigned char* packet, unsigned size)
{
	ENetPacket *ePacket = enet_packet_create(packet,
		size,
		ENET_PACKET_FLAG_UNSEQUENCED);
	enet_host_broadcast(m_server, 0, ePacket);
	enet_host_flush(m_server);
}

void Server::disconnectClient(unsigned clientIndex)
{
	enet_peer_disconnect(m_clientArray[clientIndex].peer, 0);
	ENetEvent eEvent;
	// Allow client to disconnect gently for three secounds.
	// This has to be done differently later, otherwise server freezes for three seconds.
	// Timer could be started on the main loop and use force disconnect there.
	while(enet_host_service (m_server, &eEvent, 3000) > 0)
		switch (eEvent.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(eEvent.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			puts("Disconnection succeeded.");
			return;
		default:
			break;
		}
	enet_peer_reset(eEvent.peer);
}

unsigned char* Server::createGameSetupPacket(unsigned playerAmount, unsigned id, size_t &size)
{
	// Send data that has to be only send once.
	size = sizeof(MessageIdentifiers) + sizeof(unsigned)*2;
	unsigned char* data = (unsigned char*)malloc(size);
	size_t index = 0;

	*((MessageIdentifiers*)(&data[index])) = Start;
	index += sizeof(MessageIdentifiers);

	*((unsigned*)(&data[index])) = id;
	index += sizeof(unsigned);
	
	*((unsigned*)(&data[index])) = playerAmount;
	index += sizeof(unsigned);

	return data;
}

unsigned char* Server::createGameUpdatePacket(std::vector<Gladiator> &gladiators,
	size_t &size)
{
	// Game update-packet sends data that needs to be constantly updated
	// Bullet data should be send separately

	size = m_updateSize;  // gladiator data
	unsigned char* data = m_updateMemory;
	size_t index = 0;

	*((MessageIdentifiers*)(&data[index])) = Update;
	index += sizeof(MessageIdentifiers);

	for (unsigned i = 0; i < gladiators.size(); i++)
	{
		*((float*)(&data[index])) = gladiators[i].m_position.x;
		index += sizeof(float);

		*((float*)(&data[index])) = gladiators[i].m_position.y;
		index += sizeof(float);
		
		*((float*)(&data[index])) = gladiators[i].m_velocity.x;
		index += sizeof(float);
		
		*((float*)(&data[index])) = gladiators[i].m_velocity.y;
		index += sizeof(float);
		
		*((float*)(&data[index])) = gladiators[i].m_rotation;
		index += sizeof(float);
	}

	return data;
}

void Server::receiveMovePacket(unsigned char* data, unsigned id)
{
	// we know the message type, so no need for id.
	size_t index = sizeof(MessageIdentifiers);

	// these should be saved on playerinput
	printf("%d\n", m_gladiatorVector.size());
	m_clientArray[id].input.x = *((float*)(&data[index]));
	index += sizeof(float);

	m_clientArray[id].input.y = *((float*)(&data[index]));
	index += sizeof(float);

}

void Server::physics()
{
	for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		// Check if players are moving in maximun speed and allow input only if they arent.
		if (m_gladiatorVector[i].m_velocity.x < 10)
		{
			m_gladiatorVector[i].m_velocity.x += m_clientArray[i].input.x;
		}

		if (m_gladiatorVector[i].m_velocity.y < 10)
		{
			m_gladiatorVector[i].m_velocity.y += m_clientArray[i].input.y;
		}

		// Move players according to speed.
		m_gladiatorVector[i].m_position.x += m_gladiatorVector[i].m_velocity.x;
		m_gladiatorVector[i].m_position.y += m_gladiatorVector[i].m_velocity.y + m_world.gravity;

		// Check some limits and keep our brave warriors inside battlefield
		if (m_gladiatorVector[i].m_position.x > m_world.limits.x)
		{
			m_gladiatorVector[i].m_position.x = m_world.limits.x - 10;
			m_gladiatorVector[i].m_velocity.x = -m_gladiatorVector[i].m_velocity.x;
		}
		else if (m_gladiatorVector[i].m_position.x < 0)
		{
			m_gladiatorVector[i].m_position.x = 10;
			m_gladiatorVector[i].m_velocity.x = -m_gladiatorVector[i].m_velocity.x;
		}

		if (m_gladiatorVector[i].m_position.y > m_world.limits.y)
		{
			m_gladiatorVector[i].m_position.y = m_world.limits.y - 10;
			m_gladiatorVector[i].m_velocity.y = -m_gladiatorVector[i].m_velocity.y;
		}
		else if (m_gladiatorVector[i].m_position.y < 0)
		{
			m_gladiatorVector[i].m_position.y = 10;
			m_gladiatorVector[i].m_velocity.y = -m_gladiatorVector[i].m_velocity.y;
		}

		// Slow velocities down with every update.
		m_gladiatorVector[i].m_velocity.x = 0.9 *m_gladiatorVector[i].m_velocity.x;
		m_gladiatorVector[i].m_velocity.y = 0.9 *m_gladiatorVector[i].m_velocity.y;


	}
}

#endif