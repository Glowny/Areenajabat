#pragma once
#if defined(ARENA_SERVER)
#include <queue>
#include <vector>
#include <enet\enet.h>
struct Message
{
	unsigned playerID;
	unsigned char* data;
};

struct Client
{
	unsigned playerID;
	ENetPeer* peer;
};

// Help: http://enet.bespin.org/Tutorial.html#Connecting
class Network
{
	public:
		Network();
		~Network();
		
		void startServer(std::queue<Message>* messageQueue, unsigned address,
			unsigned port, unsigned clientAmount);
		
		uint32_t getConnectedPlayerAmount()
		{
			return m_clientVector.size();
		}
		void checkEvent();
		// Todo: add flush for packet sending.
		void sendPacket(unsigned char* data, uint32_t size, 
			unsigned clientIndex, bool reliable = true);
		void broadcastPacket(unsigned char* data, uint32_t size,
			bool reliable = true);
		void disconnectClient(unsigned clientIndex);
		void setPlayerIds(std::vector<unsigned int> idVector);
	private:
		ENetHost* m_server;
		std::queue<Message>* m_messageQueue;
		std::vector<Client> m_clientVector;
		unsigned idGiver;
};
#endif