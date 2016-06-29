#pragma once
#if defined(ARENA_CLIENT)
#include <queue>
#include <vector>
#include <enet\enet.h>

// Help: http://enet.bespin.org/Tutorial.html#Connecting
class Network
{
	public:
		Network();
		~Network();
		
		void connectServer(char* address, unsigned port);
		void checkEvent();
		void sendPacket(unsigned char* data, size_t size, bool reliable = true);

		void disconnect();
		void setMessageQueue(std::queue<unsigned char*> *messageQueue);
	private:
		void initializeENet();
		ENetHost* createENetClient();
		ENetHost* m_client;
		ENetPeer* m_peer;
		std::queue<unsigned char*>* m_messageQueue;

};
#endif