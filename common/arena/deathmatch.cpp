#include "deathmatch.h"
#include <algorithm>
#include <time.h>
#include <stdlib.h>

namespace arena
{
	DeathMatch::DeathMatch(
		int32_t index, 
		Scoreboard* m_scoreboard,
		Physics* m_physics,
		std::vector<Player>* m_players, 
		GameMap* m_map, 
		std::vector<const NetworkEntity*>*
		m_synchronizationList,
		int maxScore
	) : 
		GameMode(
			index,
			m_scoreboard,
			m_physics,
			m_players,
			m_map,
			m_synchronizationList
		)
	{
		DeathMatch::maxScore = maxScore;
		DeathMatch::distribution = std::uniform_int_distribution<int>(0, 11);

		GameMode::init_m_tickets = 3;
	}
	DeathMatch::~DeathMatch()
	{
		printf("Deleted DeathMatch\n");
	}

	bool DeathMatch::isEnd()
	{
		int countDead = 0;
		bool checker = false;
		for (auto player = m_scoreboard->m_playerScoreVector.begin(); player != m_scoreboard->m_playerScoreVector.end(); player++)
		{
			if (player->m_score >= maxScore)
				checker = true;
			if (player->m_tickets <= 0)
				countDead++;
		}
		if (checker || countDead == m_scoreboard->m_playerScoreVector.size() - 1) {
			if (!msgChecker) {
				generateMessage();
				msgChecker = true;
			}
			return true;
		}
		msgChecker = false;
		return false;
	}

	bool DeathMatch::canAttack(Gladiator*, Gladiator* target)
	{
		return true && target->m_alive;
	}

	float DeathMatch::calculateScore(Gladiator*, Gladiator*)
	{
		return 10.0f;
	}

	struct greater_than
	{
		inline bool operator() (const PlayerScore& struct1, const PlayerScore& struct2)
		{
			return (struct1.m_score > struct2.m_score);
		}
	};

	void DeathMatch::generateMessage()
	{
		int maxScore = -1;
		std::stringstream ss;
		std::vector<PlayerScore> m_playerScoreVector(m_scoreboard->m_playerScoreVector);
		std::sort(m_playerScoreVector.begin(), m_playerScoreVector.end(), greater_than());
		maxScore = m_playerScoreVector.at(0).m_score;
		for (auto player = m_playerScoreVector.begin(); player != m_playerScoreVector.end(); player++)
		{
			ss << "Player " << player->m_playerID << ": " << player->m_score;
			if (player->m_score == maxScore) {
				ss << "  -- The Winner";
			}
			ss << "=";
		}
		endMessage = ss.str();
	}

	glm::vec2* DeathMatch::spawnLocation(Player*)
	{
		int number = 0;
		while(true)
		{
			number = distribution(generator);
			if (std::find(locationHistory.begin(), locationHistory.end(), number) != locationHistory.end()) {
				/* contains number*/
				continue;
			}
			else {
				/* does not contain number*/
				locationHistory.push_back(number);
				break;
			}
		}
		if (locationHistory.size() >= 6)
		{
			locationHistory.pop_front();
		}
		glm::vec2* target;
		target = &m_map->m_playerSpawnLocations[number];
		return target;
	}

	void DeathMatch::spawnPlayers()
	{
		for (auto m_player = m_players->begin(); m_player < m_players->end(); m_player++)
		{
			DeathMatch::spawnPlayer(&(*m_player));
		}
	}

	void DeathMatch::respawnPlayers()
	{
		for (auto m_player = m_players->begin(); m_player < m_players->end(); m_player++)
		{
			DeathMatch::respawnPlayer(&(*m_player));
		}
	}

	void DeathMatch::spawnPlayer(Player* m_player)
	{
		GameMode::resetPlayer(m_player);

		glm::vec2* target = DeathMatch::spawnLocation(m_player);
		m_physics->setGladiatorPosition(*target, m_player->m_gladiator->getEntityID());
	}

	void DeathMatch::respawnPlayer(Player* m_player)
	{
		GameMode::resetPlayer(m_player);

		glm::vec2* target = DeathMatch::spawnLocation(m_player);
		m_physics->setGladiatorPosition(*target, m_player->m_gladiator->getEntityID());
		m_physics->applyImpulseToGladiator(glm::vec2(1, 1), m_player->m_gladiator->getEntityID());

		// HAX, USE EVENTHANDLER
		NetworkEntity* entity = new NetworkEntity(NetworkEntityType::RespawnPlayer, 0);
		entity->setEntityID(m_player->m_clientIndex);
		m_synchronizationList->push_back(entity);
		entity->destroy();

		// update position because of gravity.
		m_synchronizationList->push_back(m_player->m_gladiator);
	}

}