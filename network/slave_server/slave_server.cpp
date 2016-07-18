#include "slave_server.h"
#include <common/arena/game_map.h>
#include <common/arena/weapons.h>
#include <common/arena/gladiator.h>
#include <common/arena/playerController.h>
#include <common/arena/scoreboard.h>
#include <common/game_vars.h>
#include "../game_host.h"
#include "../client_listener.h"

namespace arena
{

	SlaveServerClientListener::SlaveServerClientListener(GameHost& host) : m_host(host) { }

    SlaveServerClientListener::~SlaveServerClientListener() { }

    void SlaveServerClientListener::onClientConnected(uint32_t clientIndex, ENetPeer* from, double timestamp)
    {
        fprintf(stderr, "SlaveServerClientListener::onClientConnect(), idx = %d joined\n", clientIndex);
        BX_UNUSED(clientIndex, from, timestamp);

		m_host.registerPlayer(clientIndex);
	}
    
    void SlaveServerClientListener::onClientDisconnected(uint32_t clientIndex, ENetPeer* from, double timestamp)
    {
        BX_UNUSED(from, timestamp);
        fprintf(stderr, "SlaveServerClientListener::onClientDisconnected(), idx = %d disconnected\n", clientIndex);

		m_host.unregisterPlayer(clientIndex);
    }

    SlaveServer::SlaveServer(const char* const gamemodeName) :
		m_startTime(0),
		m_totalTime(0.0),
		m_server(&m_sendQueue),
		m_host(GameVars(gamemodeName)),
		m_clientListener(m_host)
	{
		m_receiveQueue.reserve(InitialNetworkQueueSize);
		m_sendQueue.reserve(InitialNetworkQueueSize);
        m_server.addClientListener(&m_clientListener);
	}

	void SlaveServer::queueIncoming(Packet* packet, ENetPeer* from)
	{
		m_receiveQueue.push_back(PacketEntry{ from, packet });
	}

	void SlaveServer::initialize()
	{
		m_startTime = bx::getHPCounter();
	}

	void SlaveServer::step()
	{
		// More time stuff
		int64_t currentTime = bx::getHPCounter();
		const int64_t time = currentTime - m_startTime;
		m_startTime = currentTime;
		const double frequency = (double)bx::getHPFrequency();
		// seconds
		float lastDeltaTime = float(time * (1.0f / frequency));
		m_totalTime += lastDeltaTime;

		// incoming packets
		for (PacketEntry& packetEntry : m_receiveQueue)
		{
			Packet* packet = packetEntry.m_packet;
			ENetPeer* from = packetEntry.m_peer;

			if (packet->getType() <= PacketTypes::Disconnect)
			{
				m_server.processPacket(packet, from, m_totalTime);
			}
			else if (packet->getType() == PacketTypes::GameInput)
			{
				GameInputPacket* inputPacket = (GameInputPacket*)packet;

				const uint32 index	= m_server.findExistingClientIndex(from, inputPacket->m_clientSalt, inputPacket->m_challengeSalt);
				const float32 x		= inputPacket->x;
				const float32 y		= inputPacket->y;

				m_host.processInput(index, x, y);
			}
			else if (packet->getType() == PacketTypes::GameShoot)
			{
				GameShootPacket* shootPacket = (GameShootPacket*)packet;

				const uint32 index			= m_server.findExistingClientIndex(from, shootPacket->m_clientSalt, shootPacket->m_challengeSalt);
				const bool shootingFlags	= true;
				const float32 angle			= shootPacket->m_angle;
					
				m_host.processShooting(index, shootingFlags, angle);
			
			}
			else
			{
				fprintf(stderr, "Not implemented packets %d\n", packet->getType());
			}

			destroyPacket(packet);
		}

		// all packets have been destroyed so it's safe to clear send queue now
		m_receiveQueue.clear();

		// let the disconnects happen if they happen
		m_server.checkTimeout(m_totalTime);

		// sync the servers game state after we have processed
		// the incoming packets
		std::vector<const NetworkEntity*> synchronizationList;
		m_host.getSynchronizationList(synchronizationList);

		// Packet all entities to be updated
		// TODO: pack all entities on one packet?
		GameUpdatePacket *gladiatorUpdatePacket = NULL;
		GameSpawnBulletsPacket* spawnBulletsPacket = NULL;
		
		for (const NetworkEntity* entity : synchronizationList)
		{
			const NetworkEntityType type = entity->type();

			switch (type)
			{
			case NetworkEntityType::Gladiator:
			{
				Gladiator* gladiator = (Gladiator*)entity;
				// TODO: Pack all the gladiator data on one packet.
				if (gladiatorUpdatePacket == NULL)
				{
					gladiatorUpdatePacket = new GameUpdatePacket;
					gladiatorUpdatePacket->m_playerAmount = 0;
				}
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_position = gladiator->m_position;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_velocity = gladiator->m_velocity;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_rotation = gladiator->m_rotation;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_ownerId = gladiator->m_ownerId;

				gladiatorUpdatePacket->m_playerAmount++;
				break;
			}
			case NetworkEntityType::Player:
				// Send Player data, cast.
				break;

			case NetworkEntityType::Projectile:
			{
				// Send new projectile creation data. Bullets should not be synced after creation, 
				// as the data will be too late to make any difference.
				Bullet* bulletSpawn = (Bullet*)entity;
				if (spawnBulletsPacket == NULL)
				{
					spawnBulletsPacket = new GameSpawnBulletsPacket;
					spawnBulletsPacket->m_bulletAmount = 0;
				}
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_position = bulletSpawn->m_position;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_rotation = bulletSpawn->m_rotation;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_type = bulletSpawn->m_type;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_creationDelay = bulletSpawn->m_creationDelay;
				spawnBulletsPacket->m_bulletAmount++;
				break;
			}

			case NetworkEntityType::Weapon:
				// weapon switch synch.
				// Send Weapon data, cast.
				break;
			case NetworkEntityType::Map:
			{// Send map data. This data is only send once per game.
				GamePlatformPacket* packet = new GamePlatformPacket();
				GameMap* mapEntity = (GameMap*)entity;
				for (auto platform : mapEntity->m_platformVector)
				{
					packet->m_platform.m_type = platform.type;
					packet->m_platform.m_vertexAmount = platform.vertices.size();
					for (unsigned i = 0; i < platform.vertices.size(); i++)
					{
						packet->m_platform.m_vertexArray[i] = platform.vertices[i];
					}
					for (Player& player : m_host.players())
					{
						m_server.sendPacketToConnectedClient(player.m_clientIndex, packet, m_totalTime);
					}
				}
				break;
			}
			case NetworkEntityType::Null:
			default:
				DEBUG_PRINT("sync error! trying to sync entity with no type over the network!");
				break;
			}

			for (Player& player : m_host.players()) 
			{
				if (spawnBulletsPacket != NULL)
				{ 
					m_server.sendPacketToConnectedClient(player.m_clientIndex, spawnBulletsPacket, m_totalTime);
				}
				if (gladiatorUpdatePacket != NULL)
				{ 
					m_server.sendPacketToConnectedClient(player.m_clientIndex, gladiatorUpdatePacket, m_totalTime);
				}
			}

		}
	}

	void SlaveServer::initializeRound(unsigned playerAmount)
	{
		// Load map. TODO: Use filesystem.
		m_host.loadMap("coordinatesRawData.dat");

		// Add gladiators.
		std::vector<Player>& players = m_host.players();
		Physics& physics			 = m_host.physics();
		GameMap map					 = m_host.map();
		unsigned i					 = 0;

		// TODO: for debugging.
		for (auto it = players.begin(); it != players.end(); ++it)
		{
			// Create.
			Player* player = &*it;
			Gladiator* gladiator = new Gladiator;

			gladiator->m_physicsId = physics.addGladiator(map.m_playerSpawnLocations[i]);
			gladiator->m_weapon = new WeaponGladius;
			player->m_gladiator = gladiator;

			// Register.
			m_host.registerEntity(gladiator);
		}

		// Send start packets
		GameSetupPacket* packet = new GameSetupPacket;
		packet->m_playerAmount = playerAmount;

		for (Player& player : players) {
			m_server.sendPacketToConnectedClient(player.m_clientIndex, packet, m_totalTime);
		}

		m_last_time = bx::getHPCounter();
	}

	bool SlaveServer::startRound(unsigned playerAmount)
	{
		initializeRound(playerAmount);
		return true;
	}

	void SlaveServer::updateRound()
	{
		applyPlayerInputs();

		m_host.tick(getDeltaTime());
	}
	void SlaveServer::applyPlayerInputs()
	{
		m_host.applyPlayerInputs();
	}

	float64 SlaveServer::getDeltaTime()
	{
		const int64 currentTime = bx::getHPCounter();
		const int64 time = currentTime - m_last_time;

		m_last_time = currentTime;

		const float64 frequency = (float64)bx::getHPFrequency();

		return float64(time * (1.0f / frequency));
	}

	#pragma		region Unused
	//void SlaveServer::sendCharactersData()
	//{
	//	GameUpdatePacket* updatePacket	= new GameUpdatePacket;
	//	auto& players					= m_host.players();
	//	uint32 i						= 0;

	//	updatePacket->m_playerAmount = players.size();

	//	// Get data.
	//	for (auto it = players.begin(); it != players.end(); ++it)
	//	{
	//		CharacterData characterData;
	//		characterData.m_position			= it->m_gladiator->m_position;
	//		characterData.m_velocity			= it->m_gladiator->m_position;
	//		characterData.m_rotation			= it->m_gladiator->m_rotation;
	//		updatePacket->m_characterArray[i++]	= characterData;
	//	}

	//	// Send i guess..
	//	for (uint32 i = 0; i < players.size(); i++) 
	//	{
	//		m_server.sendPacketToConnectedClient(players[i].m_clientIndex, updatePacket, this->m_totalTime);
	//	}
	//}

	//void SlaveServer::createAllBullets()
	//{
	//	auto& players = m_host.players();

	//	for (auto it = players.begin(); it != players.end(); ++it) createBullets(&*it);
	//}

	//void SlaveServer::createBullets(Player* player)
	//{
	//	if (!player->m_playerController->shootFlag) return;

	//	player->m_playerController->shootFlag = true;

	//	Weapon* weaponPointer = player->m_gladiator->m_weapon;

	//	std::vector<Bullet> bullets = weaponPointer->createBullets(
	//		player->m_playerController->aimAngle,
	//		player->m_gladiator->m_position);

	//	GameSpawnBulletsPacket* packet = new GameSpawnBulletsPacket;
	//	packet->m_bulletAmount = uint8(bullets.size());

	//	Physics& physics = m_host.physics();

	//	for (unsigned i = 0; i < bullets.size(); i++)
	//	{
	//		packet->m_bulletSpawnArray[i].m_creationDelay = bullets[i].m_creationDelay;
	//		packet->m_bulletSpawnArray[i].m_position = bullets[i].m_position;
	//		packet->m_bulletSpawnArray[i].m_rotation = bullets[i].m_rotation;
	//		packet->m_bulletSpawnArray[i].m_type = bullets[i].m_type;

	//		physics.addBullet(bullets[i].m_position, bullets[i].m_impulse, player->m_gladiator->m_physicsId);
	//	}

	//	pushPacketToQueue(packet);
	//}

	//void SlaveServer::pushPacketToQueue(Packet* packet)
	//{
	//	// TODO: When threads are implemented, add mutex.
	//	m_outPacketQueue->push(packet);
	//}
#pragma endregion

	Packet* SlaveServer::getPacketFromQueue()
	{
		// TODO: When threads are implemented, add mutex.
		Packet* packet = m_inPacketQueue->front();
		m_inPacketQueue->pop();
		return packet;
	}

	std::vector<PacketEntry>& SlaveServer::getSendQueue()
	{
		return m_sendQueue;
	}
}