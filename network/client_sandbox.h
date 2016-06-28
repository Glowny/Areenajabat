#pragma once
#if defined(ARENA_CLIENT)

#include <enet\enet.h>
#include <string>
#include <queue>
#include "Enumerations.h"
#include <SFML/Graphics.hpp>
struct vec2 // temporary
{
	double x;
	double y;

};

struct PlatformPoints
{
	// array of points that determine platform
	vec2 position;
	
	sf::Vertex* platform;
	size_t m_size;
	
};

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
	std::vector<PlatformPoints> m_points;
private:
	float ennakointi;
	unsigned m_myId;
	void connect(char* address, unsigned port);
	void initializeENet();
	ENetHost* createENetClient();
	void sendPacket(unsigned char* data, unsigned size);
	void checkEvent();
	MessageIdentifier getID(unsigned char* data);
	void openUpdatePackage(unsigned char* data);
	void openPlatformPackage(unsigned char* data); 
	void openStartPackage(unsigned char* data);
	unsigned char* createMovePacket(size_t &size, double velocity_x,
		double velocity_y);
	std::queue<unsigned char*> messages;
	ENetHost* m_client;
	ENetEvent m_eEvent;
	ENetPeer* m_peer;
};

#endif