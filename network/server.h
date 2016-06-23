#pragma once
#include <enet\enet.h>
#include <queue>
#include <vector>
struct Message {};

struct PlayerInput
{
	float xDir= 0;
	float yDir= 0;
};


struct Client
{
	ENetPeer* peer;
	std::queue<unsigned char*> messageQueue;
	PlayerInput input;
};

struct WorldPhysics
{
	struct vec2
	{
		float x, y;
	};
	float gravity;
	vec2 limits;
};


struct Gladiator
{
	unsigned m_id;
	float m_position_x;
	float m_position_y;
	float m_velocity_x;
	float m_velocity_y;
	float m_rotation;
};

struct Bullet
{
	// This data is send only when bullet is created
	// Client will calculate physics for it.
	// For different kind of bullets do we need id to accompany?
	unsigned m_id;
	float m_position_x;
	float m_position_y;
	float m_rotation;
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
	void checkEvent();
	void sendPacket(unsigned char* packet, unsigned size, unsigned clientIndex);
	void broadcastPacket(unsigned char* packet, unsigned size);
	void disconnectClient(unsigned clientIndex);
	void physics();
	unsigned char* createGameSetupPacket(unsigned playerAmount, unsigned id, size_t &size);
	unsigned char* createGameUpdatePacket(std::vector<Gladiator> &gladiators,
								std::vector<Bullet> &bullets, size_t &size);
	void receiveMovePacket(unsigned char* data, unsigned id);

	WorldPhysics m_world;
	std::vector<Gladiator> m_gladiatorVector;
	std::vector<Bullet> m_bulletVector;
	ENetHost* m_server;
	Client m_clientArray[10];	// raise if needed. Clients could be peers instead of events
	unsigned m_clientAmount;
};