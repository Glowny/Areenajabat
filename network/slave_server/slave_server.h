#pragma once

#include "common/arena/arena_packet.h"
#include <queue>
#include <map>
#include <bx/timer.h>

#include "common/forward_declare.h"
#include "common/types.h"
FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Server)
#define TIMESTEP 0.016f

namespace arena
{

	struct ArenaPlatform;
	struct Weapon;
	struct PlayerController;
	struct Gladiator;

	class SlaveServer final
	{
	public:
		SlaveServer();
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
	};
}