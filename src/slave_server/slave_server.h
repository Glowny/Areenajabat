#pragma once

#include "common\arena\game_map.h"
#include "common\arena\physics.h"
#include "common\arena\weapons.h"
#include "common\arena\arena_packet.h"
#include <queue>
#include <map>

#include "common\forward_declare.h"
#include "common\types.h"

FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Server)

/*
	Wraps the server instances that the master manages.
	One machine can run multiple instances of game
	server.
*/
struct PlayerController
{
	glm::ivec2 m_movementDirection;
	float m_jumpDirection;
};

struct Gladiator
{
	Gladiator() { m_alive = true; m_hitpoints = 100; }
	unsigned m_physicsId;
	glm::vec2 m_position;
	glm::vec2 m_velocity;
	float m_rotation;
	arena::Weapon m_weapon;
	int m_hitpoints;
	bool m_alive;
};

struct Player
{
	// id used by master server to communicate with client.
	unsigned m_networkId;
	PlayerController* m_playerController;
	Gladiator* m_gladiator;
};



namespace arena
{
	class SlaveServer final
	{
	public:
		SlaveServer(const uint32 address, const uint32 port);
		~SlaveServer();

		void initialize();

		bool startLobby();
		bool stop();
		bool startRound();
		bool returnToLobby();


		std::queue<Packet*> *m_inPacketQueue;
		std::queue<Packet*> *m_outPacketQueue;
	private:
		Server* const m_instance;
		
		// Access player data by network id.
		std::map<unsigned, Player*> playerMap;

		// Set map and add gladiators.
		void initializeRound();
		
		// Update positions
		void sendCharactersData();

		// Create new bullet
		void createBullets(float angle, Gladiator* gladiator);

		// Network:
		// Send packet to client(s). Packet can be created anywhere.
		void pushPacketToQueue(Packet* packet);
		// Read all packets from client(s) in queue and act according to each packet.
		void handleIncomingPackets();
		// Handle a single packet
		void handleSinglePacket(Packet* packet);
		// Get oldest packet from client.
		Packet* getPacketFromQueue();

		GameMap m_map;
		Physics m_physics;

	};
}