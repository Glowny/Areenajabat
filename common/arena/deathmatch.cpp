#include "deathmatch.h"
#include <algorithm>

namespace arena
{
	DeathMatch::DeathMatch(Scoreboard* m_scoreboard, int maxScore) : GameMode(m_scoreboard)
	{
		DeathMatch::maxScore = maxScore;
	}
	DeathMatch::~DeathMatch()
	{
		printf("Deleted DeathMatch\n");
	}

	bool DeathMatch::isEnd()
	{
		for (auto player = m_scoreboard->m_playerScoreVector.begin(); player != m_scoreboard->m_playerScoreVector.end(); player++)
		{
			bool checker = false;
			if (player->m_score >= maxScore)
				checker = true;
			if (player->m_tickets < 0)
				checker = true;
			if (checker) {
				generateMessage();
				return checker;
			}
		}
		return false;
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

}