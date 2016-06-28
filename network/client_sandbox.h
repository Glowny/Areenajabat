#pragma once
#if defined(ARENA_CLIENT)

#include <enet\enet.h>
#include <string>
#include <queue>
#include "Enumerations.h"


struct Gladiator 
{
	unsigned id;
	double m_position_y;
	double m_position_x;
	double m_velocity_x;
	double m_velocity_y;
	double m_movedir_x;
	double m_movedir_y;
	double m_rotation;
};

class Client
{
public:
	Client() {};
	~Client() {};
	void start(char* address, unsigned port);
	std::vector<Gladiator> m_gladiatorVector;
private:

	unsigned m_myId;
	void connect(char* address, unsigned port);
	void initializeENet();
	ENetHost* createENetClient();
	void sendPacket(unsigned char* data, unsigned size);
	void checkEvent();
	MessageIdentifier getID(unsigned char* data);
	void openUpdatePackage(unsigned char* data);
	void openStartPackage(unsigned char* data);
	unsigned char* createMovePacket(size_t &size, float velocity_x,
									float velocity_y);
	std::queue<unsigned char*> messages;
	ENetHost* m_client;
	ENetEvent m_eEvent;
	ENetPeer* m_peer;
};

#endif