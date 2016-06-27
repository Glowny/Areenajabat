#if defined(ARENA_SERVER)
#include "server.h"
#include <stdio.h>
#include <assert.h>
#include "Enumerations.h"
#include <bx/timer.h>
#include "Serializer.h"

Server::Server()
{
	
	b2Vec2 platform3[6];
	platform3[0].Set(0,0);
	platform3[1].Set(0,500);
	platform3[2].Set(250,650);
	platform3[3].Set(750,650);
	platform3[4].Set(1000,500);
	platform3[5].Set(1000,0);


}

Server::~Server()
{

}

void Server::start(unsigned address, unsigned port, unsigned playerAmount)
{
	m_messageQueue = new std::queue<Message>;
	network.startServer(m_messageQueue, address, port, playerAmount);
	
	// wait for players..
	while (network.getConnectedPlayerAmount() < playerAmount)
		network.checkEvent();

	for (int i = 0; i < playerAmount; i++)
	{
		PlayerInput input;
		input.moveDir.x = 0;
		input.moveDir.y = 0;
		m_playerVector.push_back(input);
	}

	for (int i = 0; i < playerAmount; i++)
	{
		Gladiator glad;
		glad.m_id = physics.addGladiator(i*50,50);
		
		m_gladiatorVector.push_back(glad);
	}
	

	// reserve space for update message.
	//m_updateSize = sizeof(MessageIdentifier) + sizeof(float) * 5 * m_gladiatorVector.size();;
	//m_updateMemory =(unsigned char*)malloc(m_updateSize);
	
	for(unsigned i = 0; i < playerAmount; i++)
		{
			size_t size;
			unsigned char* data = createGameSetupPacket(playerAmount, i, size);
			network.sendPacket(data, size, i);
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
			physics.update();

			for (int i = 0; i < m_gladiatorVector.size(); i++)
			{
				m_gladiatorVector[i].m_position = physics.getGladiatorPosition(i);
				m_gladiatorVector[i].m_velocity = physics.getGladiatorVelocity(i);
			}
		}
		network.checkEvent();

		// Handle messages
		for (unsigned playerIndex= 0; playerIndex < playerAmount; playerIndex++)
		{
			while (m_messageQueue->size() != 0)
			{
				unsigned char* data;
				data = m_messageQueue->front().data;
				unsigned id = m_messageQueue->front().clientID;
				// handle data here
				receiveMovePacket(data, id);
				free(data);
				m_messageQueue->pop();

			}
		}
		
		if(updateNetwork > 0.1)
		{
			size_t size;
			printf("updating network \n");
			unsigned char *data = createGameUpdatePacket(m_gladiatorVector, size);
			network.broadcastPacket(data, size, false);
			updateNetwork = 0;
		}
	}
}




unsigned char* Server::createGameSetupPacket(unsigned playerAmount, unsigned id, size_t &size)
{
	/*
	// Send data that has to be only send once.
	size = sizeof(MessageIdentifier) + sizeof(unsigned)*2;
	unsigned char* data = (unsigned char*)malloc(size);
	size_t index = 0;

	*((MessageIdentifier*)(&data[index])) = Start;
	index += sizeof(MessageIdentifier);

	*((unsigned*)(&data[index])) = id;
	index += sizeof(unsigned);
	
	*((unsigned*)(&data[index])) = playerAmount;
	index += sizeof(unsigned);

	return data;
	*/
}

unsigned char* Server::createGameUpdatePacket(std::vector<Gladiator> &gladiators,
	size_t &size)
{
	/*
	// Game update-packet sends data that needs to be constantly updated
	// Bullet data should be send separately

	size = m_updateSize;  // gladiator data
	unsigned char* data = m_updateMemory;
	size_t index = 0;

	*((MessageIdentifier*)(&data[index])) = Update;
	index += sizeof(MessageIdentifier);

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
	*/
}

void Server::receiveMovePacket(unsigned char* data, unsigned id)
{
	DataType dataTypes[3]{ unsignedInt, Float, Float };
	deSerialize(dataTypes, 3, data, &m_playerVector[id].moveDir.x, &m_playerVector[id].moveDir.y);
}


#endif