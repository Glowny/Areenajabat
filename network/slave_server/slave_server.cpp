#include "slave_server.h"
#include <common/arena/weapons.h>
#include <common/arena/gladiator.h>
#include <common/arena/playerController.h>
#include <common/arena/scoreboard.h>

using namespace arena;

SlaveServer::SlaveServer()
{
}

void SlaveServer::addPlayer(uint64_t salt)
{
	Player* player = new Player();
	player->m_clientSalt = salt;
	player->m_playerController = new PlayerController();
	m_playerMap.insert(std::pair<uint64_t, Player*>(salt, player));
}

void SlaveServer::initializeRound(unsigned playerAmount)
{
	// Load map. TODO: Use filesystem.
	//m_map.loadMapFromFile("coordinatesRawData.dat");

	// Add gladiators.
	unsigned i = 0;
	for (std::map<unsigned, Player*>::const_iterator mapIterator = m_playerMap.begin();
		mapIterator != m_playerMap.end(); ++mapIterator)
	{
		Gladiator gladiator;
		gladiator.m_physicsId = m_physics.addGladiator(m_map.m_playerSpawnLocations[i]);
		gladiator.m_weapon = new WeaponGladius;
		i++;
	}

	// Send start packets
	GameSetupPacket* packet = new GameSetupPacket;
	packet->m_playerAmount = playerAmount;
	pushPacketToQueue(packet);

	m_last_time = bx::getHPCounter();
}

bool SlaveServer::startRound(unsigned playerAmount)
{
	initializeRound(playerAmount);
	return true;
}

void SlaveServer::updateRound()
{
	handleIncomingPackets();
	applyPlayerInputs();
	updatePhysics();
}
void SlaveServer::applyPlayerInputs()
{
	for (std::map<unsigned, Player*>::const_iterator mapIterator = m_playerMap.begin();
		mapIterator != m_playerMap.end(); ++mapIterator)
	{
		unsigned physicsId = mapIterator->second->m_gladiator->m_physicsId;
		glm::ivec2 moveDirection = mapIterator->second->m_playerController->m_movementDirection;
		glm::vec2 currentVelocity = m_physics.getGladiatorVelocity(physicsId);
		
		if (currentVelocity.x < 250 && currentVelocity.x > -250)
		{ 
			glm::vec2 force;
			force.x = moveDirection.x *1500.0f;
			m_physics.applyForceToGladiator(force, physicsId);
			
		}
	}
	createAllBullets();
}

float SlaveServer::getDeltaTime()
{
	int64_t currentTime = bx::getHPCounter();
	const int64_t time = currentTime - m_last_time;
	m_last_time = currentTime;
	const double frequency = (double)bx::getHPFrequency();

	return float(time * (1.0f / frequency));
}
void SlaveServer::updatePhysics()
{
	if ((m_physics.updateTimer += getDeltaTime()) > TIMESTEP)
	{
		m_physics.update();
	}	
}

void SlaveServer::sendCharactersData()
{
	GameUpdatePacket* updatePacket = new GameUpdatePacket;
	//TODO: Does playeramount need to be serialized?
	updatePacket->m_playerAmount = m_playerMap.size();
	
	unsigned i = 0;
	for (std::map<unsigned, Player*>::const_iterator mapIterator = m_playerMap.begin();
		mapIterator != m_playerMap.end(); ++mapIterator)
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

void SlaveServer::createAllBullets()
{
	for (std::map<unsigned, Player*>::const_iterator mapIterator = m_playerMap.begin();
		mapIterator != m_playerMap.end(); ++mapIterator)
	{
		createBullets(mapIterator->second);
	}
}

void SlaveServer::createBullets(Player* player)
{
	if (player->m_playerController->shootFlag == false)
		return;
	player->m_playerController->shootFlag = true;

	Weapon* weaponPointer = player->m_gladiator->m_weapon;
	std::vector<Bullet> bullets = weaponPointer->createBullets(player->m_playerController->aimAngle, 
		player->m_gladiator->m_position);

	GameSpawnBulletsPacket* packet = new GameSpawnBulletsPacket;
	packet->m_bulletAmount = bullets.size();
	for (unsigned i = 0; i < bullets.size(); i++)
	{
		packet->m_bulletSpawnArray[i].m_creationDelay	 = bullets[i].m_creationDelay;
		packet->m_bulletSpawnArray[i].m_position		 = bullets[i].m_position;
		packet->m_bulletSpawnArray[i].m_rotation		 = bullets[i].m_rotation;
		packet->m_bulletSpawnArray[i].m_type			 = bullets[i].m_type;
		
		m_physics.addBullet(bullets[i].m_position, bullets[i].m_impulse, player->m_gladiator->m_physicsId);
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
			Player* player = m_playerMap[inputPacket->m_clientSalt];

			player->m_playerController->m_movementDirection.x = inputPacket->x;
			player->m_playerController->m_movementDirection.y = inputPacket->y;
			break;
		}

		case PacketTypes::GameShoot:
		{
			GameShootPacket* shootPacket = (GameShootPacket*)packet;
			Player* player = m_playerMap[shootPacket->m_clientSalt];
			player->m_playerController->shootFlag = true;
			player->m_playerController->aimAngle = shootPacket->m_angle;
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
