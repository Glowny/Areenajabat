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
#include "..\game_host.h"

FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Server)

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Player)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, ClientData)

namespace arena
{

    struct SlaveServerClientListener : public ClientListener
    {
		GameHost& m_host;

		SlaveServerClientListener(GameHost& host);

        ~SlaveServerClientListener() override;

        void onClientConnected(uint32_t clientIndex, ENetPeer* from, double timestamp) override;

        // everything after this call becomes invalid data and should not be used
        // the peer will be nullified after this and clientIndex will be given to another client
        // after they connect
        void onClientDisconnected(uint32_t clientIndex, ENetPeer* from, double timestamp) override;
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

		// Get oldest packet from client.
		Packet* getPacketFromQueue();

		int64_t m_last_time;
		
		GameHost m_host;
	};
}