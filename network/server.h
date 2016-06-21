#pragma once
#include <enet\enet.h>
#include <queue>
struct Message {};

struct Client
{
	ENetEvent eClient;
	std::queue<Message> messageQueue;
		
};

class Server 
{
public:
	Server();
	~Server();
	void start(unsigned port, unsigned playerAmount);
private:
	void initializeENet();
	ENetHost* createENetServer(unsigned address, unsigned port, unsigned playerAmount);
	int checkEvent();
	void sendPacket(char* testString, unsigned size, unsigned clientIndex);
	void broadcastPacket(char* testString, unsigned size);
	void disconnectClient(unsigned clientIndex);
	ENetHost* m_server;
	ENetPeer* m_clientArray[10];	// raise if needed. Clients could be peers instead of events
	unsigned m_clientAmount;
};