#include "slave_server.h"
#include <common/arena/weapons.h>
#include <common/arena/gladiator.h>
#include <common/arena/playerController.h>
#include <common/arena/scoreboard.h>
#include "../game_host.h"

namespace arena
{

	SlaveServer::SlaveServer() :
		m_startTime(0),
		m_totalTime(0.0),
		m_server(&m_sendQueue)
	{
		m_receiveQueue.reserve(InitialNetworkQueueSize);
		m_sendQueue.reserve(InitialNetworkQueueSize);
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

		// update physics and shit fill sendQueue
	}

	void SlaveServer::addPlayer(ClientData* const clientData)
	{
		m_host->registerPlayer(clientData);
	}

	void SlaveServer::initializeRound(unsigned playerAmount)
	{
		// Load map. TODO: Use filesystem.
		m_host->loadMap("coordinatesRawData.dat");

		// Add gladiators.
		std::vector<Player>& players = m_host->players();
		Physics& physics			= m_host->physics();
		GameMap map					= m_host->map();
		unsigned i					= 0;

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

		m_host->tick(getDeltaTime());
	}
	void SlaveServer::applyPlayerInputs()
	{
		m_host->applyPlayerInputs();

		createAllBullets();
	}

	float64 SlaveServer::getDeltaTime()
	{
		const int64 currentTime = bx::getHPCounter();
		const int64 time = currentTime - m_last_time;

		m_last_time = currentTime;

		const float64 frequency = (float64)bx::getHPFrequency();

		return float64(time * (1.0f / frequency));
	}

	void SlaveServer::sendCharactersData()
	{
		GameUpdatePacket* updatePacket	= new GameUpdatePacket;
		auto& players					= m_host->players();
		unsigned i						= 0;

		updatePacket->m_playerAmount = players.size();

		for (auto it = players.begin(); it != players.end(); ++it)
		{
			Player* player = &*it;

			CharacterData characterData;
			characterData.m_position = player->m_gladiator->m_position;
			characterData.m_velocity = player->m_gladiator->m_position;
			characterData.m_rotation = player->m_gladiator->m_rotation;
			updatePacket->m_characterArray[i] = characterData;

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
		packet->m_bulletAmount = uint8(bullets.size());

		Physics& physics = m_host->physics();

		for (unsigned i = 0; i < bullets.size(); i++)
		{
			packet->m_bulletSpawnArray[i].m_creationDelay = bullets[i].m_creationDelay;
			packet->m_bulletSpawnArray[i].m_position = bullets[i].m_position;
			packet->m_bulletSpawnArray[i].m_rotation = bullets[i].m_rotation;
			packet->m_bulletSpawnArray[i].m_type = bullets[i].m_type;

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

			const uint64 salt = inputPacket->m_clientSalt;
			const float32 x = inputPacket->x;
			const float32 y = inputPacket->y;

			m_host->processInput(salt, x, y);
			break;
		}

		case PacketTypes::GameShoot:
		{
			// nugget niggas
			GameShootPacket* shootPacket = (GameShootPacket*)packet;

			const uint64 salt = shootPacket->m_clientSalt;
			const bool shootingFlags = true;
			const float32 angle = shootPacket->m_angle;

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

	std::vector<PacketEntry>& SlaveServer::getSendQueue()
	{
		return m_sendQueue;
	}
}