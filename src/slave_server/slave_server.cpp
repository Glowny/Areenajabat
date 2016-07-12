#include "slave_server.h"
using namespace arena;

void SlaveServer::initializeRound()
{
	// Load map. TODO: Use filesystem.
	//m_map.loadMapFromFile("coordinatesRawData.dat");

	// Add gladiators.
	unsigned i = 0;
	for (std::map<unsigned, Player*>::const_iterator mapIterator = playerMap.begin();
		mapIterator != playerMap.end(); ++mapIterator)
	{
		Gladiator gladiator;
		gladiator.m_physicsId = m_physics.addGladiator(m_map.m_playerSpawnLocations[i]);
		i++;
	}
}

void SlaveServer::sendCharactersData()
{
	GameUpdatePacket* updatePacket = new GameUpdatePacket;
	//TODO: Does playeramount need to be serialized?
	updatePacket->m_playerAmount = playerMap.size();
	
	unsigned i = 0;
	for (std::map<unsigned, Player*>::const_iterator mapIterator = playerMap.begin();
		mapIterator != playerMap.end(); ++mapIterator)
	{
		CharacterData characterData;
		characterData.m_position = mapIterator->second->m_gladiator->m_position;
		characterData.m_velocity = mapIterator->second->m_gladiator->m_position;
		characterData.m_rotation = mapIterator->second->m_gladiator->m_rotation;
		updatePacket->m_characterArray[i] = characterData;
		i++;
	}
	pushPacketToQueue(updatePacket);
}

void SlaveServer::createBullets(float angle, Gladiator* gladiator)
{
	std::vector<Bullet> bullets = gladiator->m_weapon.createBullets(angle, gladiator->m_position);

	GameSpawnBulletsPacket* packet = new GameSpawnBulletsPacket;
	packet->m_bulletAmount = bullets.size();
	for (unsigned i = 0; i < bullets.size(); i++)
	{
		packet->m_bulletSpawnArray[i].m_creationDelay	 = bullets[i].m_creationDelay;
		packet->m_bulletSpawnArray[i].m_position		 = bullets[i].m_position;
		packet->m_bulletSpawnArray[i].m_rotation		 = bullets[i].m_rotation;
		packet->m_bulletSpawnArray[i].m_type			 = bullets[i].m_type;
	}
	pushPacketToQueue(packet);
}

void SlaveServer::pushPacketToQueue(Packet* packet)
{
	// TODO: When threads are implemented, add mutex.
	m_outPacketQueue->push(packet);
}

void SlaveServer::handleIncomingPackets()
{
	while (!m_inPacketQueue->empty())
	{
		handleSinglePacket(getPacketFromQueue());
	}
}

void SlaveServer::handleSinglePacket(Packet* packet)
{
	switch (packet->getType())
	{
		case PacketTypes::GameInput:
		{
			GameInputPacket* inputPacket = (GameInputPacket*)packet;
			//TODO: Set ID on master server.
			inputPacket->m_id;
			inputPacket->x;
			inputPacket->y;
			break;
		}

		case PacketTypes::GameShoot:
		{
			GameShootPacket* shootPacket = (GameShootPacket*)packet;
			shootPacket->m_id;
			shootPacket->m_angle;
			break;
		}

		default:
			printf("Game: Packet id: %d unknown.\n", packet->getType());
			break;
	}
}

Packet* SlaveServer::getPacketFromQueue()
{
	// TODO: When threads are implemented, add mutex.
	Packet* packet = m_inPacketQueue->front();
	m_inPacketQueue->pop();
	return packet;
}
