#pragma once
#if defined(ARENA_CLIENT)

#include <glm\glm.hpp>
#include <queue>
#include "GamePackets.h"
#include "NetworkClient.h"
#include <SFML/Graphics.hpp>

struct LiveBullet
{
	glm::vec2 position;
	glm::vec2 velocity;
	sf::RectangleShape* m_rectangle;
	
};

struct BulletHit
{
	glm::vec2 position;
	float lifeTime;
	float currentTime;
};

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

	// Gameplay stuff
	void updateGameplay();

	// Data send from server
	
	// platforms send by server
	std::vector<Platform> m_points;
	bool noMoreBullets;
	std::vector<BulletInputData> m_bulletVector;
	std::queue<unsigned char*> m_messageQueueIn;

	// Data send to server
	glm::vec2 m_movedir;
	float aimAngle;
	// Messages server send.
	
	// game objects
	std::vector<GladiatorData> m_gladiatorVector;
	std::vector<LiveBullet> m_liveBulletVector;
	std::vector<BulletHit> m_bulletHitVector;
	ScoreBoard m_scoreBoard;
	bool drawScoreBoard;
	// Network low level	
	Network m_network;

	// Id used for broadcasted messages
	unsigned m_myId;

	// SFML stuff for window, draw and clock.
	sf::RenderWindow* m_window;
	sf::View m_view;
	sf::Clock m_networkClock;
	sf::Clock m_physicsClock;
	sf::Clock m_timerClock;
	// Vertex data about platforms used for drawing by SFML
	std::vector<std::vector<sf::Vertex>> m_vertexes;
	sf::RectangleShape m_rectangle;
	sf::RectangleShape m_bulletRectangle;
	sf::Font font;
	sf::Text hpText;
	sf::Text m_scoreBoardText;
	std::string formatScoreBoardText(ScoreBoard &scoreBoard);


};

#endif