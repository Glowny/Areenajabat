#include "slave_server.h"
#include <common/arena/weapons.h>
#include <common/arena/gladiator.h>
#include <common/arena/playerController.h>
#include <common/arena/scoreboard.h>
#include "../game_host.h"

using namespace arena;

SlaveServer::SlaveServer()
{
	// TODO: init game host.
}

void SlaveServer::addPlayer(uint64_t salt)
{
	(void)salt;
	// TODO: register player with the host.
	//m_host->registerPlayer(clientData);
}

void SlaveServer::initializeRound(unsigned playerAmount)
{
	// Load map. TODO: Use filesystem.
	m_host->loadMap("coordinatesRawData.dat");

	// Add gladiators.
	std::vector<Player>& players = m_host->players();
	Physics& physics			 = m_host->physics();
	GameMap map					 = m_host->map();
	unsigned i					 = 0;

	// TODO: for debugging.
	for (auto it = players.begin(); it != players.end(); ++it)
	{
		// Create.
		Player* player			= &*it;
		Gladiator* gladiator	= new Gladiator;

		gladiator->m_physicsId	= physics.addGladiator(map.m_playerSpawnLocations[i]);
		gladiator->m_weapon		= new WeaponGladius;
		player->m_gladiator		= gladiator;

		// Register.
		m_host->registerEntity(gladiator);
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
	m_host->applyPlayerInputs();

	createAllBullets();
}

float SlaveServer::getDeltaTime()
{
	const int64 currentTime = bx::getHPCounter();
	const int64 time		= currentTime - m_last_time;
	
	m_last_time = currentTime;

	const float64 frequency = (float64)bx::getHPFrequency();

	return float(time * (1.0f / frequency));
}
void SlaveServer::updatePhysics()
{
	// TODO: not needed i guess?

	//if ((m_physics.updateTimer += getDeltaTime()) > TIMESTEP)
	//{
	//	m_physics.update();
	//}	
}

void SlaveServer::sendCharactersData()
{
	GameUpdatePacket* updatePacket	= new GameUpdatePacket;
	auto& players					= m_host->players();
	unsigned i						= 0;

	updatePacket->m_playerAmount	= players.size();
	
	for (auto it = players.begin(); it != players.end(); ++it)
	{
		Player* player = &*it;

		CharacterData characterData;
		characterData.m_position			= player->m_gladiator->m_position;
		characterData.m_velocity			= player->m_gladiator->m_position;
		characterData.m_rotation			= player->m_gladiator->m_rotation;
		updatePacket->m_characterArray[i]	= characterData;
		
		i++;
	}

	pushPacketToQueue(updatePacket);
}

void SlaveServer::createAllBullets()
{
	auto& players = m_host->players();

	for (auto it = players.begin(); it != players.end(); ++it) createBullets(&*it);
}

void SlaveServer::createBullets(Player* player)
{
	if (!player->m_playerController->shootFlag) return;
	
	player->m_playerController->shootFlag = true;

	Weapon* weaponPointer = player->m_gladiator->m_weapon;
	
	std::vector<Bullet> bullets = weaponPointer->createBullets(
		player->m_playerController->aimAngle, 
		player->m_gladiator->m_position);

	GameSpawnBulletsPacket* packet = new GameSpawnBulletsPacket;
	packet->m_bulletAmount = bullets.size();
	
	Physics& physics = m_host->physics();

	for (unsigned i = 0; i < bullets.size(); i++)
	{
		packet->m_bulletSpawnArray[i].m_creationDelay	 = bullets[i].m_creationDelay;
		packet->m_bulletSpawnArray[i].m_position		 = bullets[i].m_position;
		packet->m_bulletSpawnArray[i].m_rotation		 = bullets[i].m_rotation;
		packet->m_bulletSpawnArray[i].m_type			 = bullets[i].m_type;
		
		physics.addBullet(bullets[i].m_position, bullets[i].m_impulse, player->m_gladiator->m_physicsId);
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
			
			const uint64 salt	= inputPacket->m_clientSalt;
			const float32 x		= inputPacket->x;
			const float32 y		= inputPacket->y;

			m_host->processInput(salt, x, y);
			break;
		}

		case PacketTypes::GameShoot:
		{
			GameShootPacket* shootPacket = (GameShootPacket*)packet;

			const uint64 salt			= shootPacket->m_clientSalt;
			const bool shootingFlags	= true;
			const float32 angle			= shootPacket->m_angle;
			
			m_host->processShooting(salt, shootingFlags, angle);
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
