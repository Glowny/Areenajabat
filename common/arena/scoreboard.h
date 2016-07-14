#pragma once
#include <vector>
namespace arena
{

	struct PlayerScore
	{
		int m_score;
		int m_tickets;
	};

	struct Scoreboard
	{
		unsigned m_flagHolder;
		std::vector<PlayerScore> m_playerScoreVector;
	};
}