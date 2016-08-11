#include "deathmatch.h"

namespace arena
{
	DeathMatch::DeathMatch(Scoreboard* m_scoreboard, int maxScore, int maxKills) : GameMode(m_scoreboard)
	{
		DeathMatch::maxScore = maxScore;
		DeathMatch::maxKills = maxKills;
	}
	DeathMatch::~DeathMatch()
	{
		printf("Deleted DeathMatch\n");
	}

	bool DeathMatch::isEnd()
	{
		for (auto player = m_scoreboard->m_playerScoreVector.begin(); player != m_scoreboard->m_playerScoreVector.end(); player++)
		{
			if (player->m_score >= maxScore)
				return true;
			if (player->m_kills >= maxKills)
				return true;
			if (player->m_tickets < 0)
				return true;
		}
		return false;
	}

}