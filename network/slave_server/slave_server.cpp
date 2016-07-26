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

    void onGameStart()
    {
        fprintf(stderr, "On game start\n");
		
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
        m_host.startSession();
        m_host.e_gameStart += onGameStart;
	}

	void SlaveServer::queueIncoming(Packet* packet, ENetPeer* from)
	{
		m_receiveQueue.push_back(PacketEntry{ from, packet });
	}

    void SlaveServer::broadcast(Packet* packet)
    {
        std::vector<Player>& players = m_host.players();

        for (Player& player : players)
        {
            m_server.sendPacketToConnectedClient(player.m_clientIndex, packet, m_totalTime);
        }
    }

    void SlaveServer::addListener(ClientListener* listener)
    {
        m_server.addClientListener(listener);
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
				m_host.processInput(index, inputPacket->m_input, inputPacket->m_aimAngle);
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

        m_host.tick(lastDeltaTime);

		// sync the servers game state after we have processed
		// the incoming packets
		std::vector<const NetworkEntity*> synchronizationList;
		m_host.getSynchronizationList(synchronizationList);

		// Packet all entities to be updated
		// TODO: pack all entities on one packet?
		GameUpdatePacket* gladiatorUpdatePacket = NULL;

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
					gladiatorUpdatePacket = (GameUpdatePacket*)createPacket(PacketTypes::GameUpdate);
					gladiatorUpdatePacket->m_playerAmount = 0;
				}
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_position = *gladiator->m_position;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_velocity = *gladiator->m_velocity;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_aimAngle = gladiator->m_aimAngle;
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
					spawnBulletsPacket = (GameSpawnBulletsPacket*)createPacket(PacketTypes::GameSpawnBullets);
					spawnBulletsPacket->m_bulletAmount = 0;
				}
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_position = *bulletSpawn->m_position;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_rotation = bulletSpawn->m_rotation;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_type = bulletSpawn->m_type;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_id = bulletSpawn->m_bulletId;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_creationDelay = bulletSpawn->m_creationDelay;
				spawnBulletsPacket->m_bulletAmount++;
				break;
			}

			case NetworkEntityType::Weapon:
				// weapon switch synch.
				// Send Weapon data, cast.
				break;

			case NetworkEntityType::BulletHit:
			{
				BulletHit* hit = (BulletHit*)entity;
				GameDamagePlayerPacket* packet = (GameDamagePlayerPacket*)createPacket(PacketTypes::GameDamagePlayer);
				packet->m_damageAmount = hit->m_damageAmount;
				packet->m_hitPosition = hit->m_hitPosition;
				packet->m_hitDirection = hit->m_hitDirection;
				packet->m_targetID = hit->m_targetPlayerId;
				broadcast(packet);
				break;
			}
			case NetworkEntityType::Map:
            {
               
                GameMap* mapEntity = (GameMap*)entity;

                for (auto platform : mapEntity->m_platformVector)
                {
                    // Send map data. This data is only send once per game.
                    GamePlatformPacket* packet = (GamePlatformPacket*)createPacket(PacketTypes::GamePlatform);
                    packet->m_platform.m_type = platform.type;
                    packet->m_platform.m_vertexAmount = uint8_t(platform.vertices.size());
                    for (unsigned i = 0; i < platform.vertices.size(); i++)
                    {
                        packet->m_platform.m_vertexArray[i] = platform.vertices[i];
                    }

                    broadcast(packet);
                }
				
				// send data about player amount and clients id.
				std::vector<Player>& players = m_host.players();

				for (Player& player : players)
				{
					GameSetupPacket* setupPacket = (GameSetupPacket*)createPacket(PacketTypes::GameSetup);
					setupPacket->m_playerAmount = (int32_t)m_host.players().size();
					setupPacket->m_clientIndex = player.m_clientIndex;
					m_server.sendPacketToConnectedClient(player.m_clientIndex, setupPacket, m_totalTime);
				}
				
				// send data about gladiators and their ids.

				GameCreateGladiatorsPacket *gladiatorsCreatePacket = (GameCreateGladiatorsPacket*)createPacket(PacketTypes::GameCreateGladiators);
				gladiatorsCreatePacket->m_playerAmount = players.size();
				for (unsigned i = 0; i < players.size(); i++)
				{
					gladiatorsCreatePacket->m_characterArray[i].m_position = *players[i].m_gladiator->m_position;
					gladiatorsCreatePacket->m_characterArray[i].m_velocity = *players[i].m_gladiator->m_velocity;
					gladiatorsCreatePacket->m_characterArray[i].m_aimAngle = players[i].m_gladiator->m_aimAngle;
					gladiatorsCreatePacket->m_characterArray[i].m_ownerId = players[i].m_gladiator->m_ownerId;
				}
				broadcast(gladiatorsCreatePacket);

       
                break;
            }
			case NetworkEntityType::Null:
			default:
				DEBUG_PRINT("sync error! trying to sync entity with no type over the network!");
				break;
			}

		}
            if (spawnBulletsPacket != NULL)
			{ 
                broadcast(spawnBulletsPacket);
			}
			if (gladiatorUpdatePacket != NULL)
			{ 
                broadcast(gladiatorUpdatePacket);
			}

	}

	float64 SlaveServer::getDeltaTime()
	{
		const int64 currentTime = bx::getHPCounter();
		const int64 time = currentTime - m_last_time;

		m_last_time = currentTime;

		const float64 frequency = (float64)bx::getHPFrequency();

		return float64(time * (1.0f / frequency));
	}

	std::vector<PacketEntry>& SlaveServer::getSendQueue()
	{
		return m_sendQueue;
	}
}