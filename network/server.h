#pragma once
#if defined(ARENA_SERVER)
#include <enet\enet.h>
#include <queue>
#include <vector>
#include <Box2D\Box2D.h>

struct vec2
{
	float x, y;
};

// Data client needs + input from players.
struct Client
{
	ENetPeer* peer;
	std::queue<unsigned char*> messageQueue;
	vec2 input;
};

// Temporary physics for the world, replace with Box2D.
struct WorldPhysics
{
	float gravity = 9.81;
	vec2 limits;
};
struct Platform
{
	b2ChainShape m_shape;
	b2BodyDef m_bodydef;
	b2Body* m_body;
	b2FixtureDef m_fixtureDef;
};

struct Gladiator
{
	// Update constantly
	unsigned m_id;
	vec2 m_position;
	vec2 m_velocity;
	float m_rotation;

	b2Body* m_body;
};

struct Bullet
{
	// This data is send only when bullet is created.
	// Client will calculate clientside physics for bullet.
	// Server calculates serverside physics, and notifies clients on hit.

	unsigned m_id;
	float m_position_x;
	float m_position_y;
	float m_rotation;

	b2BodyDef m_body;
};

class Server 
{
public:
	Server();
	~Server();
	void start(unsigned port, unsigned playerAmount);
	
private:
	// Networking low level.
	void initializeENet();
	ENetHost* createENetServer(unsigned address, unsigned port, unsigned playerAmount);
	void checkEvent();
	void sendPacket(unsigned char* packet, unsigned size, unsigned clientIndex);
	void broadcastPacket(unsigned char* packet, unsigned size);
	void disconnectClient(unsigned clientIndex);
	
	// Networking game related.
	unsigned char* createGameSetupPacket(unsigned playerAmount, unsigned id, size_t &size);
	unsigned char* createGameUpdatePacket(std::vector<Gladiator> &gladiators, size_t &size);
	void receiveMovePacket(unsigned char* data, unsigned id);
	
	// physics.
	void physics();
	WorldPhysics m_world;
	b2World* m_b2DWorld; 

	// Networking low level.
	ENetHost* m_server;
	unsigned m_clientAmount;
	Client m_clientArray[10];	// raise if needed to.

	// Space reserved for gameupdatePacket.
	size_t m_updateSize;
	unsigned char* m_updateMemory;

	// Game entities.
	std::vector<Platform> m_platformVector;
	std::vector<Gladiator> m_gladiatorVector;
	std::vector<Bullet> m_bulletVector;
};

#endif