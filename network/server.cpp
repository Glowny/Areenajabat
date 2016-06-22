#include "server.h"
#include <stdio.h>
#include <assert.h>
#include "messageIdentifiers.h"
Server::Server()
{
	m_clientAmount = 0;
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
	Gladiator glad;
	glad.m_id = 1;
	glad.m_position_x = 50;
	glad.m_position_y = 50;
	glad.m_velocity_x = 0;
	glad.m_velocity_y = 0;
	glad.m_rotation = 17;
	m_gladiatorVector.push_back(glad);

	float deltaTime = 0;


	while (true)
	{
		deltaTime += 0.05;
		if (deltaTime > 20000)
		{
			printf("position: (%f,%f) velocity(%f,%f) \n", m_gladiatorVector[0].m_position_x, m_gladiatorVector[0].m_position_y,
				m_gladiatorVector[0].m_velocity_x, m_gladiatorVector[0].m_velocity_y);
			deltaTime = 0;
			physics();
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
		
		size_t size;
		unsigned char *data = createGameUpdatePacket(m_gladiatorVector, m_bulletVector, size);
		broadcastPacket(data, size);
		free(data); // CreateGameUpdatePacket reserves space.
	}
}

// Help: http://enet.bespin.org/Tutorial.html#Connecting

void Server::physics()
{
	for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		m_gladiatorVector[i].m_position_x += m_gladiatorVector[i].m_velocity_x;
		m_gladiatorVector[i].m_position_y += m_gladiatorVector[i].m_velocity_y;
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
		// save data at this point and then destory packet.
		// Data is not saved atm.
		//m_clientArray[id].messageQueue.push(EEvent.packet->data);
		receiveMovePacket(EEvent.packet->data);

		enet_packet_destroy(EEvent.packet);
		break;
	}
	case ENET_EVENT_TYPE_DISCONNECT:
		printf("%s disconnected.\n", EEvent.peer->data);
		EEvent.peer->data = NULL;
		
	}

}

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
		ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(m_server, 0, ePacket);
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
		}
	enet_peer_reset(eEvent.peer);
}

unsigned char* Server::createGameSetupPacket(unsigned playerAmount)
{
	// Send data that has to be only send once.
	return NULL;
}

unsigned char* Server::createGameUpdatePacket(std::vector<Gladiator> &gladiators,
	std::vector<Bullet> &bullets, size_t &size)
{
	// Game update-packet sends data that needs to be constantly updated
	// Bullet data should be send separately

	size = sizeof(MessageIdentifiers) + sizeof(size_t) + sizeof(float) * bullets.size() // id, amount of bullets, bullet data.
		+ sizeof(float) * 5 * gladiators.size(); // gladiator data
	unsigned char* data = (unsigned char*)malloc(size);
	size_t index = 0;

	*((MessageIdentifiers*)(&data[index])) = Update;
	index += sizeof(MessageIdentifiers);

	*((size_t*)(&data[index])) == bullets.size();
	index += sizeof(size_t);
	
	for (unsigned i = 0; i < gladiators.size(); i++)
	{
		*((float*)(&data[index])) = gladiators[i].m_position_x;
		index += sizeof(float);
		*((float*)(&data[index])) = gladiators[i].m_position_y;
		index += sizeof(float);
		*((float*)(&data[index])) = gladiators[i].m_velocity_x;
		index += sizeof(float);
		*((float*)(&data[index])) = gladiators[i].m_velocity_y;
		index += sizeof(float);
		*((float*)(&data[index])) = gladiators[i].m_rotation;
		index += sizeof(float);
	}
	for (unsigned i = 0; i < bullets.size(); i++)
	{
		*((float*)(&data[index])) = bullets[i].m_position_x;
		index += sizeof(float);
		*((float*)(&data[index])) = bullets[i].m_position_y;
		index += sizeof(float);
		*((float*)(&data[index])) = bullets[i].m_rotation;
		index += sizeof(float);
	}
	return data;
}

void Server::receiveMovePacket(unsigned char* data)
{
	// we know the message type, so no need for id.
	size_t index = sizeof(MessageIdentifiers);

	m_gladiatorVector[0].m_velocity_x += *((float*)(&data[index]));
	index += sizeof(float);

	m_gladiatorVector[0].m_velocity_y += *((float*)(&data[index]));
	index += sizeof(float);

}
