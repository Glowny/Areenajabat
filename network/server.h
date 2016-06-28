#pragma once
#if defined(ARENA_SERVER)
#include <enet\enet.h>
#include <queue>
#include <vector>
#include <Box2D\Box2D.h>
#include "Network.h"
#include "Physics.h"

struct Platforma
{
	// array of points that determine platform
	vec2* m_platform;
	size_t m_size;

};

struct PlayerInput
{
	vec2 moveDir;
	// add more when needed;

};

struct Gladiator
{
	// Update constantly
	unsigned m_id;
	vec2 m_position;
	vec2 m_velocity;
	double m_rotation;

	
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

	b2BodyDef m_body;
};

class Server 
{
public:
	Server();
	~Server();
	void start(unsigned address, unsigned port, unsigned playerAmount);
	
private:
	// Networking game related.
	unsigned char* createGameSetupPacket(unsigned playerAmount, unsigned id, size_t &size);
	unsigned char* createGameUpdatePacket(std::vector<Gladiator> &gladiators, size_t &size);
	size_t m_updateSize;  //update packet size wont change during gameloop. 
	unsigned char* m_updateMemory; // memory set for update packet.

	void handleClientMessages();
	void handleMessage(Message &message);
	void receiveMovePacket(unsigned char* data, unsigned id);
	void receiveShootPacket(unsigned char* data, unsigned id);


	// Networking low level.
	Network network;

	// Game entities.
	Physics physics;
	
	std::vector<Gladiator> m_gladiatorVector;
	std::vector<Bullet> m_bulletVector;
	std::queue<Message>* m_messageQueue;

	std::vector<PlayerInput> m_playerVector;
};

#endif