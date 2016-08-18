#include "game_mode.h"

namespace arena
{
	GameMode::GameMode(
		int32_t index,
		Scoreboard* m_scoreboard,
		Physics* m_physics,
		std::vector<Player>* m_players,
		GameMap* m_map,
		std::vector<const NetworkEntity*>* m_synchronizationList
	)
	{
		GameMode::m_scoreboard = m_scoreboard;
		GameMode::m_physics = m_physics;
		GameMode::m_players = m_players;
		GameMode::m_map = m_map;
		GameMode::m_synchronizationList = m_synchronizationList;
		GameMode::index = index;
		waitingEndTime = 500000.0f;
		GameMode::timer = 0;
		msgChecker = false;
	}
	GameMode::GameMode(
		int32_t index,
		Scoreboard* m_scoreboard,
		std::vector<Player>* m_players
	)
	{
		GameMode::m_scoreboard = m_scoreboard;
		GameMode::m_players = m_players;
		GameMode::index = index;
		waitingEndTime = 500000.0f;
		GameMode::timer = 0;
		msgChecker = false;
	}
	GameMode::~GameMode()
	{
		printf("Deleted GameMode\n");
	}

	std::string GameMode::getEndMessage()
	{
		return GameMode::endMessage;
	}

	bool GameMode::updateEndTimer(float dt) {
		GameMode::timer += dt;
		return (GameMode::timer > GameMode::waitingEndTime);
	}

	void GameMode::resetEndTimer() {
		GameMode::timer = 0;
	}

	void GameMode::resetPlayer(Player* m_player)
	{
		m_player->m_gladiator->m_alive = true;
		m_player->m_gladiator->m_hitpoints = 100;
		delete m_player->m_gladiator->m_weapon;
		delete m_player->m_gladiator->m_grenadeWeapon;
		m_player->m_gladiator->m_weapon = new WeaponGladius;
		m_player->m_gladiator->m_grenadeWeapon = new WeaponGrenade;
	}

	void GameMode::resetScoreBoard()
	{
		for (auto it = m_scoreboard->m_playerScoreVector.begin(); it != m_scoreboard->m_playerScoreVector.end(); it++)
		{
			it->m_kills = 0;
			it->m_score = 0;
			//TODO: get the amount of tickets from initilization file
			it->m_tickets = GameMode::init_m_tickets;

		}
		GameMode::m_synchronizationList->push_back(m_scoreboard);
	}

}