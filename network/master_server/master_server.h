#pragma once
#include <stdint.h>
#include "../server.h"
#include "../slave_server/slave_server.h"
#include <unordered_map>

namespace arena
{
    class NetworkInterface;
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
        const static uint32_t MaxSlaves = 4;
	public:
		MasterServer();

		void start();

        ~MasterServer();

        void update();
    private:

        std::unordered_map<uint64_t, SlaveServer*> m_serverLookup;

        // this is shared accross all servers
        NetworkInterface* m_networkInterface;

        Server m_servers[MaxSlaves];
        SlaveServer m_slaves[MaxSlaves];
	};
}