#pragma once

#include "common/arena/physics.h"
#include "common/arena/arena_packet.h"
#include "../server.h"
#include <queue>
#include <map>
#include <bx/timer.h>
#include "../client_listener.h"
#include "common/forward_declare.h"
#include "common/types.h"
FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Server)

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, Player)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, GameHost)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, ClientData)

namespace arena
{

    struct SlaveServerClientListener : public ClientListener
    {
        ~SlaveServerClientListener() override;

        void onClientConnected(uint32_t clientIndex, ENetPeer* from, double timestamp) override;
    };

	struct ArenaPlatform;
	struct Weapon;
	struct PlayerController;
	struct Gladiator;

	class SlaveServer final
	{
        // how many packets are reserved per frame
        static const uint32_t InitialNetworkQueueSize = 256;
	public:
		SlaveServer(const char* const gamemodeName);
		~SlaveServer();

		void initialize();

		void addPlayer(const uint64 salt, const uint64 id);

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

        // 
        std::vector<PacketEntry>& getSendQueue();
	private:
        SlaveServerClientListener m_clientListener;

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

		float64 getDeltaTime();

		// Apply client imputs
		void applyPlayerInputs();

		// Update positions
		void sendCharactersData();

		// Create new bullets for all the players
		void createAllBullets();

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
		
		GameHost* m_host;
	};
}