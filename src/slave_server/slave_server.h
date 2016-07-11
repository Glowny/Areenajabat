#pragma once

#include "common\areena\physics.h"

#include "common\areena\areena_packet.h"
#include <queue>
#include <map>

#include "common\forward_declare.h"
#include "common\types.h"

FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Server)

/*
	Wraps the server instances that the master manages.
	One machine can run multiple instances of game
	server.
*/


namespace arena
{
	class SlaveServer final
	{
	public:
		SlaveServer(const uint32 address, const uint32 port);
		~SlaveServer();

		void initialize();

		bool startLobby();
		bool stop();
		bool startGame();
		bool returnToLobby();


		std::queue<Packet*> *m_inPacketQueue;
		std::queue<Packet*> *m_outPacketQueue;
	private:
		Server* const m_instance;

		// Send message to client(s). Messages can be created anywhere.
		void pushPacketToQueue(Packet* packet);

		// Read all messages from client(s) in queue and act according to each message.
		void handleIncomingPackets();
		// Handle a single message
		void handleSinglePacket(Packet* packet);

		// Get oldest message from client.
		Packet* getPacketFromQueue();

		Physics physics;
	};
}