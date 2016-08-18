#include "game_mode.h"
#include "game_mode_factory.h"

namespace arena
{

	GameMode * GameModeFactory::createGameModeFromIndex(int32_t index, Scoreboard* m_scoreboard, std::vector<Player>* m_players)
	{
		GameMode* m_gameMode;
		switch (index)
		{
		case 1:
			m_gameMode = new DeathMatch(1, m_scoreboard, 20);
			break;
		case 2:
			TeamDeathMatch* teamDeathMatch = new TeamDeathMatch(2, m_scoreboard, m_players, 2, false);
			teamDeathMatch->autoGroupTeams();
			m_gameMode = teamDeathMatch;
			break;
		}
		return m_gameMode;
	}

}