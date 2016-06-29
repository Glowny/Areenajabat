#pragma once
#if defined(ARENA_SERVER)
#include <queue>
#include <vector>
#include "NetworkServer.h"
#include "Physics.h"
#include "GamePackets.h"

struct PlayerInput
{
	glm::vec2 moveDir;
	// add more when needed;

};

struct Bullet
{
	// This data is send only when bullet is created.
	// Client will calculate clientside physics for bullet.
	// Server calculates serverside physics, and notifies clients on hit.
	unsigned m_id;
	double m_position_x;
	double m_position_y;
	double m_rotation;

};

class Server 
{
public:
	Server();
	~Server();
	void start(unsigned address, unsigned port, unsigned playerAmount);
	
private:
	// Networking game related.
	size_t m_updateSize;  //update packet size wont change during gameloop. 
	unsigned char* m_updateMemory; // memory set for update packet.

	void handleClientMessages();
	void handleMessage(Message &message);

	// Networking low level.
	Network m_network;

	// Game entities.
	Physics m_physics;
	
	std::vector<GladiatorData> m_gladiatorVector;
	std::vector<Bullet> m_bulletVector;
	std::queue<Message>* m_messageQueue;

	std::vector<PlayerInput> m_playerInputVector;
	std::vector<Platform> m_platformVector;
};

#endif