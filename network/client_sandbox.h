#pragma once
#include <enet\enet.h>
#include <string>
#include <queue>
struct Gladiator 
{
	unsigned id;
	float m_position_y;
	float m_position_x;
	float m_velocity_x;
	float m_velocity_y;
	float m_movedir_x;
	float m_movedir_y;
	float m_rotation;
};

class Client
{
public:
	Client() {};
	~Client() {};
	void start(char* address, unsigned port);
	Gladiator m_gladiator;
private:

	
	void connect(char* address, unsigned port);
	void initializeENet();
	ENetHost* createENetClient();
	void sendPacket(unsigned char* data, unsigned size);
	void checkEvent();
	void openUpdatePackage(unsigned char* data);
	unsigned char* createMovePacket(size_t &size, float velocity_x,
									float velocity_y);
	std::queue<unsigned char*> messages;
	ENetHost* m_client;
	ENetEvent m_eEvent;
	ENetPeer* m_peer;
};