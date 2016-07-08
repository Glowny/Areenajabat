#pragma once
#if defined(ARENA_SERVER)
#include <queue>
#include <vector>
#include "NetworkServer.h"
#include "Physics.h"
#include <common/GamePackets.h>
#include <common/network_interface.h>
#include <common/types.h>


struct PlayerInput
{
	glm::vec2 moveDir;
	float jumpTimer;
	// add more when needed;
};


class Server
{
public:
    static const uint32_t MaxClients = 32;

    Server();
	~Server();
	void start(const String& iniPath);
    void start(unsigned address, unsigned port, unsigned playerAmount);
private:
    arena::NetworkInterface* m_networkInterface;

    ENetPeer m_clientPeers[MaxClients];

    bool m_clientConnected[MaxClients];
    uint64_t m_clientSalt[MaxClients];

    uint64_t m_serverSalt;

    uint32_t m_clientsConnected;

	// Networking game related.
	uint32_t m_updateSize;  //update packet size wont change during gameloop. 
	unsigned char* m_updateMemory; // memory set for update packet.

	void handleClientMessages();
	void handleMessage(Message &message);
	void sendPlatformPackets();
	// Gameplay
	// Pushes back bullets on m_bulletVector.
	void createOutputBullets(std::vector<BulletInputData> &bulletInputVector, unsigned playerId);

	// Networking low level.
	Network m_network;

	// Game entities.
	Physics m_physics;

	// Game Initiazation stuff
	void createPlayerInputs(unsigned playerAmount);
	void createGladiators(unsigned playerAmount);
	void sendSetupPackets(unsigned playerAmount);

	// Game loop stuff
	void gladiatorMovement();
	void sendBulletCreationEvents();
	void sendBulletHitEvents();
	void sendGameUpdateMessages();
	//  TODO: Make different respawn system later
	void respawnDeadPlayers();

	ScoreBoard m_scoreBoard;
	std::vector<GladiatorData> m_gladiatorVector;
	std::vector<BulletOutputData> m_bulletOutputVector;
	std::queue<Message>* m_messageQueue;
	std::vector<PlayerInput> m_playerInputVector;
	std::vector<Platform> m_platformVector;

	unsigned m_playerAmount;
	void loadPlatformsFromFile(char* filename);
	bool m_run;
};

#endif