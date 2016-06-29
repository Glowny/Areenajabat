#pragma once
#if defined(ARENA_SERVER)
#include <queue>
#include <vector>
#include <enet\enet.h>
struct Message
{
	unsigned clientID;
	unsigned char* data;
};
// Help: http://enet.bespin.org/Tutorial.html#Connecting
class Network
{
	public:
		Network();
		~Network();
		
		void startServer(std::queue<Message>* messageQueue, unsigned address,
			unsigned port, unsigned clientAmount);
		
		size_t getConnectedPlayerAmount()
		{
			return m_clientVector.size();
		}
		void checkEvent();
		void sendPacket(unsigned char* data, size_t size, 
			unsigned clientIndex, bool reliable = true);
		void broadcastPacket(unsigned char* data, size_t size,
			bool reliable = true);
		void disconnectClient(unsigned clientIndex);
	
	private:
		void initializeENet();
		ENetHost* createENetServer(unsigned address, unsigned port,
			unsigned clientAmount);
		ENetHost* m_server;
		std::queue<Message>* m_messageQueue;
		std::vector<ENetPeer*> m_clientVector;
};
#endif