#pragma once

#include "common\forward_declare.h"
#include "common\arena\arena_packet.h"
#include "minini\minIni.h"
#include "common\types.h"

#include <vector>
#include "../slave_server/slave_server.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, SlaveServer)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, NetworkClient)

/*
Level 1: master - accept connections, send usr data etc,
handles client redirections to servers
Lavel 2: servers - gameplay stuff happens here
*/



namespace arena
{
	class MasterServer final
	{
	public:
		MasterServer(const String& configurationPath);

		void start();

		~MasterServer();
	private:
		void addPacketToQueue(unsigned slaveId, Packet* packet);
		Packet* getPacketFromQueue(unsigned slaveId);
		std::vector<SlaveServer*>	m_slaves;
		std::vector<NetworkClient*> m_clients;

		const uint32 m_address;
		const uint32 m_port;
	};
}