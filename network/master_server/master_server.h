#pragma once
#include <stdint.h>
#include "../server.h"
#include "../slave_server/slave_server.h"
#include <unordered_map>
#include <vector>

namespace arena
{
    class NetworkInterface;
    class Server;
}


/*
Level 1: master - accept connections, send usr data etc,
handles client redirections to servers
Lavel 2: servers - gameplay stuff happens here
*/

// TODO: Way for slaves to send packet to every client they have and to a single client.

namespace arena
{
    // Creates common network interface to be used accross all slave servers
	class MasterServer final
	{
        // number of server networks and slaves to be created
        const static uint32_t MaxGameInstances = 10;
	public:
		MasterServer();

		void start();

        ~MasterServer();

        void update();
    private:
        void processCreateLobbyPacket(CreateLobbyPacket* packet, ENetPeer* from, double timestamp);
        void processJoinLobbyPacket(JoinLobbyPacket* packet, ENetPeer* from, double timestamp);
        void processListLobbiesPacket(ListLobbiesPacket* packet, ENetPeer* from, double timestamp);

        // 
        std::unordered_map<uint64_t, uint32_t> m_clientSaltToLobbyIndex;

        using LobbySaltToIndexMap = std::unordered_map<uint64_t, uint32_t>;
        LobbySaltToIndexMap m_lobbySaltToLobbyIndex;
        // this is shared accross all servers
        NetworkInterface* m_networkInterface;

        std::vector<SlaveServer*> m_gameInstances;
        uint64_t m_instanceCreatedBy[MaxGameInstances];
        uint64_t m_lobbySalts[MaxGameInstances];
        std::string m_lobbyNames[MaxGameInstances];

        bool m_running;
	};
}