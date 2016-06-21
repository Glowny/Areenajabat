#pragma once
#include <enet\enet.h>
#include <string>
class Client
{
public:
	Client() {};
	~Client() {};
	void start(char* address, unsigned port);
private:
	void connect(char* address, unsigned port);
	void initializeENet();
	ENetHost* createENetClient();
	void sendPacket(char* testString, unsigned size);
	void checkEvent();
	ENetHost* m_client;
	ENetEvent m_eEvent;
	ENetPeer* m_peer;
};