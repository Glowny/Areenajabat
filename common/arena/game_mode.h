#pragma once

#include <string>
#include "scoreboard.h"
#include "gladiator.h"
#include <sstream>
#include "physics.h"
#include "player.h"
#include "game_map.h"

namespace arena
{
	class GameMode
	{
	public:
		GameMode(
			int32_t index, 
			Scoreboard* m_scoreboard, 
			Physics* m_physics, 
			std::vector<Player>* m_players, 
			GameMap* m_map, 
			std::vector<const NetworkEntity*>* m_synchronizationList
		);
		GameMode(
			int32_t index,
			Scoreboard* m_scoreboard,
			std::vector<Player>* m_players
		);
		virtual ~GameMode() = 0;

		virtual bool isEnd() = 0;
		virtual bool canAttack(Gladiator* shooter, Gladiator* target) = 0;
		virtual float calculateScore(Gladiator* shooter, Gladiator* target) = 0;
		virtual void spawnPlayers() = 0;
		virtual void spawnPlayer(Player* m_player) = 0;
		virtual void respawnPlayers() = 0;
		virtual void respawnPlayer(Player* m_player) = 0;

		//void setEndMessage(std::string endMessage);
		std::string getEndMessage();
		bool updateEndTimer(float dt);
		void resetEndTimer();
		void resetScoreBoard();

		void resetPlayer(Player* player);

		int32_t index;

	protected:
		std::string endMessage;
		Scoreboard* m_scoreboard;
		Physics* m_physics;
		GameMap* m_map;
		std::vector<const NetworkEntity*>* m_synchronizationList;
		std::vector<Player>* m_players;
		float waitingEndTime;
		float timer;
		bool msgChecker;
		int init_m_tickets = 5;

		//virtual glm::vec2* respawnPlayer(Player* m_player, glm::vec2* spawnLocation)= 0;
	};
}