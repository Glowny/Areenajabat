#include "team_deathmatch.h"
#include <math.h>

namespace arena
{
	TeamDeathMatch::TeamDeathMatch(Scoreboard* m_scoreboard, std::vector<Player>* m_players, int numTeams) : GameMode(m_scoreboard)
	{
		TeamDeathMatch::m_players = m_players;
		TeamDeathMatch::numTeams = numTeams;

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
		uint8_t winTeam = -1;
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
		return -1;
	}

	//assign team ID to each player
	void TeamDeathMatch::autoGroupTeams()
	{
		int numPlayer = m_players->size();
		int numPlayerInTeam = ceil( 1.0 * m_players->size() / numTeams);

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

}