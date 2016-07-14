#pragma once

#include "common/arena/physics.h"
#include "common/arena/arena_packet.h"
#include <queue>
#include <map>
#include <bx/timer.h>

#include "common/forward_declare.h"
#include "common/types.h"
#include <common/arena/game_map.h>
FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Server)
#define TIMESTEP 0.016f

namespace arena
{

	struct ArenaPlatform;
	struct Weapon;
	struct PlayerController;
	struct Gladiator;
	/*
		Wraps the server instances that the master manages.
		One machine can run multiple instances of game
		server.
	*/
	struct Player
	{
		// salt is used by master server to communicate with client.
		uint64_t m_clientSalt;
		PlayerController* m_playerController;
		Gladiator* m_gladiator;
	};



	class SlaveServer final
	{
	public:
		SlaveServer(const uint32 address, const uint32 port);
		~SlaveServer();

		void addPlayer(uint64_t salt);

		bool startLobby();
		bool stop();
		bool startRound(unsigned playerAmount);
		void updateRound();
		
		bool returnToLobby();


		std::queue<Packet*> *m_inPacketQueue;
		std::queue<Packet*> *m_outPacketQueue;
	private:
		Server* const m_instance;

		// Access player data by network id.
		std::map<unsigned, Player*> m_playerMap;

		// Set map and add gladiators.
		void initializeRound(unsigned playerAmount);

		float getDeltaTime();

		void updatePhysics();

		// Apply client imputs
		void applyPlayerInputs();

		// Update positions
		void sendCharactersData();

		// Create new bullets for all the players
		void createAllBullets();

		// Create new bullets for player
		void createBullets(Player* player);

		// Network:
		// Send packet to client(s). Packet can be created anywhere.
		void pushPacketToQueue(Packet* packet);
		// Read all packets from client(s) in queue and act according to each packet.
		void handleIncomingPackets();
		// Handle a single packet
		void handleSinglePacket(Packet* packet);
		// Get oldest packet from client.
		Packet* getPacketFromQueue();

		int64_t m_last_time;
		GameMap m_map;
		Physics m_physics;

	};
}