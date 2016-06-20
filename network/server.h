#pragma once
#include <enet\enet.h>
#include <queue>
struct Message {};

struct Client
{
	ENetHost eClient;
	std::queue<Message> messageQueue;
		
};

class Server 
{
	Server();
	~Server();
	void start(unsigned port, unsigned playerAmount);
private:
	void initializeENet();
	ENetHost* createENetServer(unsigned address, unsigned port, unsigned playerAmount);
	ENetHost* createENetClient();
	char* getData(unsigned clientIndex);

	ENetHost* m_server;
	ENetHost* m_clientArray[10];	// rise if needed. Clients could be on a struct to store events.
	unsigned m_clientAmount;
};