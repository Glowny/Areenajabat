#if defined(ARENA_SERVER)
#include "server.h"
#include <stdio.h>
#include <assert.h>
#include <bx/timer.h>

Server::Server()
{
	Platform platformPoints;
	glm::vec2 vec; vec.x = 0; vec.y = 0;
	platformPoints.points.push_back(vec);
	vec.x = 0; vec.y = 500;
	platformPoints.points.push_back(vec);
	vec.x = 250; vec.y = 650;
	platformPoints.points.push_back(vec);
	vec.x = 750; vec.y = 650;
	platformPoints.points.push_back(vec);
	vec.x = 1000; vec.y = 500;
	platformPoints.points.push_back(vec);
	vec.x = 1000; vec.y = 0;
	platformPoints.points.push_back(vec);

	Platform platformPoints2;
	vec.x = 300; vec.y = 300;
	platformPoints2.points.push_back(vec);
	vec.x = 700; vec.y = 300;
	platformPoints2.points.push_back(vec);
	vec.x = 450; vec.y = 500;
	platformPoints2.points.push_back(vec);
	vec.x = 300; vec.y = 300;
	platformPoints2.points.push_back(vec);

	
	m_physics.createPlatform(glm::vec2(0,0), platformPoints.points);
	m_physics.createPlatform(glm::vec2(0, 0), platformPoints2.points);
	m_platformVector.push_back(platformPoints);
	m_platformVector.push_back(platformPoints2);

}

Server::~Server()
{

}

void Server::start(unsigned address, unsigned port, unsigned playerAmount)
{
	m_messageQueue = new std::queue<Message>;
	m_network.startServer(m_messageQueue, address, port, playerAmount);
	
	// wait for players..
	while (m_network.getConnectedPlayerAmount() < playerAmount)
		m_network.checkEvent();

	printf("GAME STARTING!\n");

	for (unsigned i = 0; i < playerAmount; i++)
	{
		PlayerInput input;
		input.moveDir.x = 0;
		input.moveDir.y = 0;
		m_playerInputVector.push_back(input);
	}

	for (unsigned i = 0; i < playerAmount; i++)
	{
		GladiatorData glad;
		m_physics.addGladiator(i*100.0f,50.0f);
		glad.rotation = 0;
		glad.position.x = 100.0f * i;
		glad.position.y = 50.0f;
		m_gladiatorVector.push_back(glad);
	}
	
	m_updateSize = sizeof(MessageIdentifier) + sizeof(float) * 5 * m_gladiatorVector.size();
	m_updateMemory = (unsigned char*)malloc(m_updateSize);
	
	for(unsigned i = 0; i < playerAmount; i++)
		{
			size_t size;
			unsigned char* data = createSetupPacket(size,playerAmount, i);
			m_network.sendPacket(data, size, i);

			// Could be broadcasted.
			size_t size2;
			unsigned char* data2 = createPlatformPacket(size2,m_platformVector);
			m_network.sendPacket(data2, size2, i);
		}

	int64_t s_last_time = bx::getHPCounter();
	float updatePhysics = 0;
	float updateNetwork = 0;

	float timeStep = 1.0f/60.0f;

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
			for (unsigned i = 0; i < m_playerInputVector.size(); i++)
			{
				if (m_playerInputVector[i].moveDir.x != 0 || m_playerInputVector[i].moveDir.y != 0)
				{ 
					m_physics.moveGladiator(m_playerInputVector[i].moveDir.x*3000, m_playerInputVector[i].moveDir.y*3000, i);
					m_playerInputVector[i].moveDir.x = 0.0;
					m_playerInputVector[i].moveDir.y = 0.0;
				}
			}
			
			updatePhysics = 0;
			m_physics.update();

			for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
			{
				m_gladiatorVector[i].position = m_physics.getGladiatorPosition(i);
				m_gladiatorVector[i].velocity = m_physics.getGladiatorVelocity(i);
			}
		}
		m_network.checkEvent();

		handleClientMessages();
		
		if(updateNetwork > 0.1)
		{
			size_t size;
			unsigned char *data = createUpdatePacket(size, m_gladiatorVector, m_updateMemory);
			m_network.broadcastPacket(data, size, false);
			updateNetwork = 0;
		}
	}
}

void Server::handleClientMessages()
{
	while (m_messageQueue->size() != 0)
	{
		handleMessage(m_messageQueue->front());
		free(m_messageQueue->front().data);
		m_messageQueue->pop();
	}
}
void Server::handleMessage(Message &message)
{
	unsigned playerId = message.clientID;
	MessageIdentifier packetID = getMessageID(message.data);

	switch (packetID)
	{
	case ClientMove:
		openMovePacket(message.data, m_playerInputVector[playerId].moveDir);
		break;
	case ClientShoot:
	
		break;
	default:
		break;
	}
}



#endif