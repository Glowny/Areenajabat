#if defined(ARENA_SERVER)
#include "server.h"
#include <stdio.h>
#include <assert.h>
#include "Enumerations.h"
#include <bx/timer.h>
#include "Serializer.h"

Server::Server()
{
	vec2 platform[6];
	platform[0].x =0   ;platform[0].y = 0;
	platform[1].x =0   ;platform[1].y = 500;
	platform[2].x =250 ;platform[2].y = 650;
	platform[3].x =750 ;platform[3].y = 650;
	platform[4].x =1000;platform[4].y = 500;
	platform[5].x =1000;platform[5].y = 0;
	vec2 position;
	position.x = 0; position.y = 0;
	physics.createPlatform(position, 6, platform);


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
		glad.m_id = physics.addGladiator(i*100,50);
		glad.m_rotation = 0;
		glad.m_position.x = 100 * i;
		glad.m_position.y = 50;
		m_gladiatorVector.push_back(glad);
	}
	
	m_updateSize = sizeof(MessageIdentifier) + sizeof(double) * 5 * m_gladiatorVector.size();
	m_updateMemory = (unsigned char*)malloc(m_updateSize);
	
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
			for (unsigned i = 0; i < m_playerVector.size(); i++)
			{
				if (m_playerVector[i].moveDir.x != 0 || m_playerVector[i].moveDir.y != 0)
				{ 
					physics.moveGladiator(m_playerVector[i].moveDir.x*300, m_playerVector[i].moveDir.y*300, i);
					m_playerVector[i].moveDir.x = 0.0;
					m_playerVector[i].moveDir.y = 0.0;
				}
			}
			
			updatePhysics = 0;
			physics.update();

			for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
			{
				m_gladiatorVector[i].m_position = physics.getGladiatorPosition(i);
				m_gladiatorVector[i].m_velocity = physics.getGladiatorVelocity(i);
			}
		}
		network.checkEvent();

		handleClientMessages();
		
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
	unsigned playerID = message.clientID;
	unsigned packetID = getID(message.data);

	switch (packetID)
	{
	case ClientMove:
		receiveMovePacket(message.data, playerID);
		break;
	case ClientShoot:
		receiveShootPacket(message.data, playerID);
	}
}

unsigned char* Server::createGameSetupPacket(unsigned playerAmount, unsigned id, size_t &size)
{
	
	DataType dataTypes[3]{ messageID, unsignedInt, unsignedInt};
	unsigned char* data = reserveSpace(dataTypes, 3, size);
	
	data = serialize(data, dataTypes, 3, Start, id, playerAmount);

	return data;
}

unsigned char* Server::createGameUpdatePacket(std::vector<Gladiator> &gladiators,
	size_t &size)
{
	
	size = m_updateSize; 
	unsigned char* data = m_updateMemory;

	DataType idDataType[1] = { messageID };
	DataType dataTypes[5]{ Float, Float, Float, Float, Float};
	double tpx, tpy, tvx, tvy, r;
	size_t index = 0;
	size_t temp_index = sizeof(MessageIdentifier);
	serializeWithIndex(data, index, idDataType, 1, Update);
	for (unsigned i = 0; i < gladiators.size(); i++)
	{
		printf("S  position: %f, %f\n velocity: %f, %f\n rotation: %f\n", m_gladiatorVector[i].m_position.x, m_gladiatorVector[i].m_position.y,
			m_gladiatorVector[i].m_velocity.x, m_gladiatorVector[i].m_velocity.y, m_gladiatorVector[i].m_rotation);

		serializeWithIndex(data, index, dataTypes, 5,  gladiators[i].m_position.x, gladiators[i].m_position.y,
			gladiators[i].m_velocity.x, gladiators[i].m_velocity.y, gladiators[i].m_rotation);
		
		deSerializeWithIndex(data, temp_index, dataTypes, 5, &tpx, &tpy, &tvx, &tvy, &r);
		printf("DS position: %f, %f\n velocity: %f, %f\n rotation: %f\n", tpx, tpy, tvx, tvy, r);

	}
	return data;
}

void Server::receiveMovePacket(unsigned char* data, unsigned playerID)
{
	DataType dataTypes[3]{ messageID, Float, Float };
	MessageIdentifier temp;
	deSerialize(data,dataTypes, 3, &temp, &m_playerVector[playerID].moveDir.x, &m_playerVector[playerID].moveDir.y);
}

void Server::receiveShootPacket(unsigned char* data, unsigned playerID)
{
	DataType dataTypes[2]{ messageID, Float };
	unsigned temp = playerID;
	temp += 0;
	float rotation;
	unsigned type;
	deSerialize(data, dataTypes, 2, &type ,&rotation);
	// create bullet
}

#endif