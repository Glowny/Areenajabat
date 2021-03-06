#pragma once
#include <vector>
#include <../common/network_entity.h>
namespace arena
{

	struct PlayerScore 
	{
		PlayerScore() :m_playerID(666), m_score(666), m_kills(666), m_tickets(666) {};
		int m_playerID;
		int m_score;
		int m_kills;
		int m_tickets;
	};

	struct Scoreboard :public NetworkEntity
	{
		Scoreboard(): NetworkEntity(NetworkEntityType::Scoreboard) 
		{
			m_flagHolder = 666;
			m_hasPhysics = false;
		};
		unsigned m_flagHolder;
		inline PlayerScore& getPlayerScore(int playerID)
		{
			for (auto it = m_playerScoreVector.begin(); it != m_playerScoreVector.end(); it++)
			{
				if (it->m_playerID == playerID)
					return *it;
			}
			return m_playerScoreVector[0];
		}
		std::vector<PlayerScore> m_playerScoreVector;
	};
}
