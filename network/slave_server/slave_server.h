#pragma once

#include "common/arena/physics.h"
#include "common/arena/arena_packet.h"
#include "../server.h"
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
        // how many packets are reserved per frame
        static const uint32_t InitialNetworkQueueSize = 256;
	public:
		SlaveServer();
		~SlaveServer();

        void initialize();

		void addPlayer(uint64_t salt);

		bool startLobby();
		bool stop();
		bool startRound(unsigned playerAmount);
		void updateRound();
		
		bool returnToLobby();

        void step();

		std::queue<Packet*> *m_inPacketQueue;
		std::queue<Packet*> *m_outPacketQueue;

        // Master server routes the packet using this function call
        // the packet must be freed eg destroyPackage() in this frame
        void queueIncoming(Packet* packet, ENetPeer* from);
	private:
        // even these are vectors, the packets are sorted correctly 
        // because the master will fill these
        std::vector<PacketEntry> m_receiveQueue;
        std::vector<PacketEntry> m_sendQueue;

        // networking backend
        Server m_server;

        int64_t m_startTime;
        double m_totalTime;
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