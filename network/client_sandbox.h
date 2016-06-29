#pragma once
#if defined(ARENA_CLIENT)

#include <glm\glm.hpp>
#include <queue>
#include "GamePackets.h"
#include "NetworkClient.h"
#include <SFML/Graphics.hpp>

class Client
{
public:
	Client() {};
	~Client() {};

	void start(char* address, unsigned port);
	
private:
	// Loop throught messages that came from server
	void handleMessages();
	// Handle single message
	void handleMessage(unsigned char* data);
	
	void draw();
	// Get controller input
	void getInput();
	// Send data to server
	void sendData();
	// Update fake physics
	void updatePhysics();

	// Data send from server
	// players
	std::vector<GladiatorData> m_gladiatorVector;
	// platforms send by server
	std::vector<Platform> m_points;
	
	// Data send to server
	glm::vec2 m_movedir;

	// Messages server send.
	std::queue<unsigned char*> m_messageQueue;
	
	// Network low level	
	Network m_network;

	// Id used for broadcasted messages
	unsigned m_myId;

	// SFML stuff for window, draw and clock.
	sf::RenderWindow* m_window;
	sf::Clock m_networkClock;
	sf::Clock m_physicsClock;
	// Vertex data about platforms used for drawing by SFML
	std::vector<std::vector<sf::Vertex>> m_vertexes;
	sf::RectangleShape m_rectangle;
};

#endif