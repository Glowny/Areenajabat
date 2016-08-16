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
	SlaveServer* server;
	SlaveServerClientListener::SlaveServerClientListener(GameHost& host) : m_host(host) { }

    SlaveServerClientListener::~SlaveServerClientListener() { }

    void SlaveServerClientListener::onClientConnected(uint32_t clientIndex, ENetPeer* from, double timestamp)
    {
        fprintf(stderr, "SlaveServerClientListener::onClientConnect(), idx = %d joined\n", clientIndex);
        BX_UNUSED(clientIndex, from, timestamp);
		m_host.registerPlayer(clientIndex);
		server->clientConnect(clientIndex);
	}
    
    void SlaveServerClientListener::onClientDisconnected(uint32_t clientIndex, ENetPeer* from, double timestamp)
    {
        BX_UNUSED(from, timestamp);
        fprintf(stderr, "SlaveServerClientListener::onClientDisconnected(), idx = %d disconnected\n", clientIndex);

		m_host.unregisterPlayer(clientIndex);
    }

    void onRoundStart()
    {
        fprintf(stderr, "On game start\n");
		server->roundStart();
    }

void SlaveServer::roundStart()
{
	std::vector<Player>& players = server->m_host.players();

	for (Player& player : players)
	{
		GameSetupPacket* setupPacket = (GameSetupPacket*)createPacket(PacketTypes::GameSetup);
		setupPacket->m_playerAmount = (int32_t)m_host.players().size();
		setupPacket->m_clientIndex = player.m_clientIndex;
		m_server.sendPacketToConnectedClient(player.m_clientIndex, setupPacket, m_totalTime);
	}

	// send data about gladiators and their ids.

	GameCreateGladiatorsPacket *gladiatorsCreatePacket = (GameCreateGladiatorsPacket*)createPacket(PacketTypes::GameCreateGladiators);
	gladiatorsCreatePacket->m_playerAmount = int32_t(players.size());
	for (unsigned i = 0; i < players.size(); i++)
	{
		gladiatorsCreatePacket->m_characterArray[i].m_position = *players[i].m_gladiator->m_position;
		gladiatorsCreatePacket->m_characterArray[i].m_velocity = *players[i].m_gladiator->m_velocity;
		gladiatorsCreatePacket->m_characterArray[i].m_aimAngle = players[i].m_gladiator->m_aimAngle;
		gladiatorsCreatePacket->m_characterArray[i].m_ownerId = players[i].m_gladiator->m_ownerId;
		gladiatorsCreatePacket->m_characterArray[i].m_id = players[i].m_gladiator->getEntityID();
	}
	broadcast(gladiatorsCreatePacket);
}
void SlaveServer::clientConnect(uint32_t clientIndex)
{
	GameSetupPacket* setupPacket = (GameSetupPacket*)createPacket(PacketTypes::GameSetup);
	setupPacket->m_playerAmount = m_host.players().size();
	setupPacket->m_clientIndex = clientIndex;
	m_server.sendPacketToConnectedClient(clientIndex, setupPacket, m_totalTime);


	GameState state = m_host.getGameState();
	switch (state)
	{

	// ||			GameStates where gameEntites exist begin.					||
	// If round is running, send connected client data about all game entities that currently exist
	case GameState::RoundRunning:
	{	
	
	}
	// If round has started but the game is on a pause.
	case GameState::Freezetime:
	{
	
	}
	// If round has started but the game is on a pause because of disconnect.
	case GameState::Timeout:
	{
		GameCreateGladiatorsPacket* packet = (GameCreateGladiatorsPacket*)createPacket(PacketTypes::GameCreateGladiators);
		// Send data about all existing game entities here.
		packet->m_playerAmount = 0;
		for (auto player = m_host.players().begin(); player != m_host.players().end(); player++)
		{

			Gladiator* glad = player->m_gladiator;
			packet->m_characterArray[packet->m_playerAmount].m_aimAngle = glad->m_aimAngle;
			packet->m_characterArray[packet->m_playerAmount].m_climbing = glad->m_climbing;
			packet->m_characterArray[packet->m_playerAmount].m_id		= glad->getEntityID();
			packet->m_characterArray[packet->m_playerAmount].m_ownerId	= glad->m_ownerId;
			packet->m_characterArray[packet->m_playerAmount].m_position = *glad->m_position;
			packet->m_characterArray[packet->m_playerAmount].m_velocity = *glad->m_velocity;
			packet->m_characterArray[packet->m_playerAmount].m_reloading= glad->m_reloading;
			packet->m_characterArray[packet->m_playerAmount].m_throwing = glad->m_throwing;
			packet->m_playerAmount++;
		}
			m_server.sendPacketToConnectedClient(clientIndex, packet, m_totalTime);
			// TODO: sent other game-entities.
	}
	// ||			GameStates where gameEntites exist end.						||


	// ||			GameStates where scoreboard and gamemode exists begin.		||
	// If a game is running, but round has not yet started, send data about scoreboard, gamemode and players to clients.
	case GameState::Running:
	{
		
	}
	// If game has been ended, dont send data about game entities, but send data about scoreboard, gamemode and players
	case GameState::Stopped:
	{
		// Send data about scoreboard and gamemode here.
		Scoreboard& board =  m_host.getScoreBoard();
		GameUpdateScoreBoardPacket* packet = (GameUpdateScoreBoardPacket*)createPacket(PacketTypes::GameUpdateScoreBoard);
		packet->m_playerAmount = (uint8_t)board.m_playerScoreVector.size();
		packet->m_scoreBoardData.m_flagHolder = board.m_flagHolder;

		for (unsigned i = 0; i < board.m_playerScoreVector.size(); i++)
		{
			packet->m_scoreBoardData.m_playerScoreArray[i].m_score = board.m_playerScoreVector[i].m_score;
			packet->m_scoreBoardData.m_playerScoreArray[i].m_tickets = board.m_playerScoreVector[i].m_tickets;
			packet->m_scoreBoardData.m_playerScoreArray[i].m_kills = board.m_playerScoreVector[i].m_kills;
			packet->m_scoreBoardData.m_playerScoreArray[i].m_playerID = board.m_playerScoreVector[i].m_playerID;
		}
		broadcast(packet);
		// TODO: gamemode send.
	}


	// ||			GameStates where scoreboard and gamemode exists end.		||
	// GameStates where only players exist begin
	// Game has not yet begun, send only data about players.
	case GameState::UnBegun:
	{
		// send data about players here.
		// TODO: is there data about players?
	}
	// ||			GameStates where only players exist end.					||
	}
}

    SlaveServer::SlaveServer(const char* const gamemodeName) :
		m_startTime(0),
		m_totalTime(0.0),
		m_server(&m_sendQueue),
		m_host(GameVars(gamemodeName)),
		m_clientListener(m_host)
	{
		server = this;
		m_receiveQueue.reserve(InitialNetworkQueueSize);
		m_sendQueue.reserve(InitialNetworkQueueSize);
        m_server.addClientListener(&m_clientListener);
        m_host.e_roundStart += onRoundStart;
        m_host.startSession();
		// Make pretty way to communicate to slave about respawn messages
		//m_host.e_respawn += [](unsigned id) { respawnPlayer(id); };
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
		const float64 frequency = bx::getHPFrequency();
		// seconds
		float64 lastDeltaTime = float64(time * (1.0f / frequency));
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
			else
				switch (packet->getType())
				{
					case PacketTypes::GameInput:
					{
						GameInputPacket* inputPacket = (GameInputPacket*)packet;
						const uint32 index = m_server.findExistingClientIndex(from, inputPacket->m_clientSalt, inputPacket->m_challengeSalt);

						m_host.processInput(index, inputPacket->m_input, inputPacket->m_aimAngle);
						break;
					}
					case PacketTypes::GameRequestMap:
					{
						GameRequestMapPacket* mapRequest = (GameRequestMapPacket*)packet;
						const uint32 index = m_server.findExistingClientIndex(from, mapRequest->m_clientSalt, mapRequest->m_challengeSalt);
						
						GameMap* mapEntity = &m_host.map();
						for (auto platform : mapEntity->m_platformVector)
						{
							// Send map data. This data is only send once per game.
							GamePlatformPacket* platformPacket = (GamePlatformPacket*)createPacket(PacketTypes::GamePlatform);
							platformPacket->m_platform.m_type = platform.type;
							platformPacket->m_platform.m_vertexAmount = int32_t(platform.vertices.size());
							for (unsigned i = 0; i < platform.vertices.size(); i++)
							{
								platformPacket->m_platform.m_vertexArray[i] = platform.vertices[i];
							}
							m_server.sendPacketToConnectedClient(index, platformPacket, m_totalTime);
						}
						break;

					}
					default:
					{
						fprintf(stderr, "Not implemented packet %d\n", packet->getType());
						break;
					}
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
				if (gladiatorUpdatePacket->m_playerAmount >= CHARACTER_MAXAMOUNT)
				{
					printf("Tried to synchronized more characters than maxAmount, fix this \n");
					return;
				}
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_position = *gladiator->m_position;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_velocity = *gladiator->m_velocity;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_aimAngle = gladiator->m_aimAngle;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_ownerId = gladiator->m_ownerId;
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_reloading = gladiator->m_reloading;
				gladiator->m_reloading = false; // Dont spam reload. TODO: move this to somewhere on game_host.
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_throwing = gladiator->m_throwing;
				gladiator->m_throwing = false; // Dont spam throwing. TODO: move this to somewhere on game_host.
				gladiatorUpdatePacket->m_characterArray[gladiatorUpdatePacket->m_playerAmount].m_climbing = gladiator->m_climbing;
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
				if (spawnBulletsPacket->m_bulletAmount >= BULLET_MAX_AMOUNT)
				{
					printf("WARNING: Too many bullets synchronized, not sending any more bullets\n");
					continue;
				}			
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_position = *bulletSpawn->m_position;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_rotation = bulletSpawn->m_rotation;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_type = bulletSpawn->m_bulletType;
				spawnBulletsPacket->m_bulletSpawnArray[spawnBulletsPacket->m_bulletAmount].m_id = bulletSpawn->getEntityID();
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
				if (hit->m_hitType == 1)
				{ 
					GameDamagePlayerPacket* packet = (GameDamagePlayerPacket*)createPacket(PacketTypes::GameDamagePlayer);
					packet->m_damageAmount = hit->m_damageAmount;
					packet->m_hitPosition = hit->m_hitPosition;
					packet->m_hitDirection = hit->m_hitDirection;
					packet->m_targetID = hit->m_targetPlayerId;
					packet->m_bulletId = hit->m_hitId;
					broadcast(packet);
				}
				else
				{
					GameBulletHitPacket* packet = (GameBulletHitPacket*)createPacket(PacketTypes::GameBulletHit);
					packet->bulletHitArray[0].m_id = hit->m_hitId;
					packet->bulletHitArray[0].m_creationDelay = 0.2f;
					packet->bulletHitArray[0].m_position = hit->m_hitPosition;
					packet->bulletHitArray[0].m_type = hit->m_hitType;
					packet->m_bulletAmount = 1;
					broadcast(packet);

				}
				break;
			}

			case NetworkEntityType::Scoreboard:
			{
				Scoreboard* ScoreboardEntity = (Scoreboard*)entity;
				GameUpdateScoreBoardPacket* packet = (GameUpdateScoreBoardPacket*)createPacket(PacketTypes::GameUpdateScoreBoard);
				packet->m_playerAmount = (uint8_t)ScoreboardEntity->m_playerScoreVector.size();
				packet->m_scoreBoardData.m_flagHolder = ScoreboardEntity->m_flagHolder;
				
				for (unsigned i = 0; i < ScoreboardEntity->m_playerScoreVector.size(); i++)
				{
					packet->m_scoreBoardData.m_playerScoreArray[i].m_score = ScoreboardEntity->m_playerScoreVector[i].m_score;
					packet->m_scoreBoardData.m_playerScoreArray[i].m_tickets = ScoreboardEntity->m_playerScoreVector[i].m_tickets;
					packet->m_scoreBoardData.m_playerScoreArray[i].m_kills = ScoreboardEntity->m_playerScoreVector[i].m_kills;
					packet->m_scoreBoardData.m_playerScoreArray[i].m_playerID = ScoreboardEntity->m_playerScoreVector[i].m_playerID;
					
				}
				broadcast(packet);
				break;
			}
			
			case NetworkEntityType::RespawnPlayer:
			{
				GameRespawnPlayerPacket* packet = (GameRespawnPlayerPacket*)createPacket(PacketTypes::GameRespawnPlayer);
				packet->m_playerID = entity->getEntityID();
				broadcast(packet);
				break;
			}

			case NetworkEntityType::Null:
			default:
				DEBUG_PRINT("sync error! trying to sync entity with no type over the network!\n");
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
			m_host.destroyEntities();

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