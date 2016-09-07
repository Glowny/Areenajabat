#include "team_deathmatch.h"
#include <math.h>

namespace arena
{
	TeamDeathMatch::TeamDeathMatch(
		int32_t index,
		Scoreboard* m_scoreboard,
		Physics* m_physics,
		std::vector<Player>* m_players,
		GameMap* m_map,
		std::vector<const NetworkEntity*>* m_synchronizationList,
		int numTeams,
		bool enableAttackTeammates
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
		TeamDeathMatch::numTeams = numTeams;
		TeamDeathMatch::enableAttackTeammates = enableAttackTeammates;

		GameMode::init_m_tickets = 8;

	}
	TeamDeathMatch::~TeamDeathMatch()
	{
		printf("Deleted Team DeathMatch\n");
	}

	bool TeamDeathMatch::isEnd()
	{
		for (int teamId = 0; teamId < numTeams; teamId++)
		{
			int count = 0;
			for (auto player = m_players->begin(); player < m_players->end(); player++)
			{
				if (player->m_gladiator->m_team == teamId)
				{
					if (m_scoreboard->getPlayerScore(player->m_gladiator->m_ownerId).m_tickets > 0)
					{
						count++;
						break;
					}
				}
			}
			if (count == 0)
			{
				if (!msgChecker)
				{
					msgChecker = true;
					generateMessage();
				}
				return true;
			}
		}
		msgChecker = false;
		return false;
	}

	bool TeamDeathMatch::canAttack(Gladiator* shooter, Gladiator* target)
	{
		return (target->m_team != shooter->m_team || enableAttackTeammates) && target->m_alive;
	}

	int TeamDeathMatch::calculateScore(Gladiator * shooter, Gladiator * target)
	{
		if (target->m_team != shooter->m_team) {
			return 10;
		}
		return 0;
	}

	struct greater_than
	{
		inline bool operator() (const PlayerScore& struct1, const PlayerScore& struct2)
		{
			return (struct1.m_score > struct2.m_score);
		}
	};


	void TeamDeathMatch::generateMessage()
	{
		int maxScore = -1;
		uint8_t winTeam = 255;
		std::stringstream ss;
		std::vector<PlayerScore> m_playerScoreVector(m_scoreboard->m_playerScoreVector);
		std::sort(m_playerScoreVector.begin(), m_playerScoreVector.end(), greater_than());
		maxScore = m_playerScoreVector.at(0).m_score;
		winTeam = findTeam(&m_playerScoreVector.at(0));
		for (auto playerScore = m_playerScoreVector.begin(); playerScore != m_playerScoreVector.end(); playerScore++)
		{
			int teamId = findTeam(&(*playerScore));
			ss << "Team : " << teamId;
			ss << " ";
			ss << "Player " << playerScore->m_playerID << ": " << playerScore->m_score;
			if (winTeam == teamId)
			{
				ss << "  -- The Winner";
			}
			ss << "=";
		}
		endMessage = ss.str();
	}

	uint8_t TeamDeathMatch::findTeam(PlayerScore* playerScore)
	{
		for (auto player = m_players->begin(); player < m_players->end(); player++)
		{
			if (player->m_gladiator->m_ownerId == playerScore->m_playerID)
			{
				return player->m_gladiator->m_team;
			}
		}
		return 255;
	}

	//assign team ID to each player
	void TeamDeathMatch::autoGroupTeams()
	{
		size_t numPlayer = m_players->size();
		int numPlayerInTeam = static_cast<int>(ceil(1.0 * numPlayer / numTeams));
		auto player = m_players->begin();
		for (int i = 0; i < numTeams; i++)
		{
			for (int j = 0; j < numPlayerInTeam; j++)
			{
				player->m_gladiator->m_team = i;
				player++;
				if (player >= m_players->end())
				{
					return;
				}
			}
		}
	}

	glm::vec2* TeamDeathMatch::spawnLocation(Player * m_player)
	{
		glm::vec2* target = NULL;
		switch (m_player->m_gladiator->m_team)
		{
		case 0:
			target = &m_map->m_playerSpawnLocations[0];
			break;
		case 1:
			target = &m_map->m_playerSpawnLocations[11];
			break;
		default:
			break;
		}
		return target;
	}

	void TeamDeathMatch::spawnPlayers()
	{
		for (auto m_player = m_players->begin(); m_player < m_players->end(); m_player++)
		{
			TeamDeathMatch::spawnPlayer(&(*m_player));
		}
	}

	void TeamDeathMatch::respawnPlayers()
	{
		for (auto m_player = m_players->begin(); m_player < m_players->end(); m_player++)
		{
			TeamDeathMatch::respawnPlayer(&(*m_player));
		}
	}

	void TeamDeathMatch::spawnPlayer(Player* m_player)
	{
		GameMode::resetPlayer(m_player);

		glm::vec2* target = TeamDeathMatch::spawnLocation(m_player);
		m_physics->setGladiatorPosition(*target, m_player->m_gladiator->getEntityID());
	}

	void TeamDeathMatch::respawnPlayer(Player* m_player)
	{
		GameMode::resetPlayer(m_player);

		glm::vec2* target = TeamDeathMatch::spawnLocation(m_player);
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