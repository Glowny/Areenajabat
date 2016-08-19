#include "game_mode.h"
#include "game_mode_factory.h"

namespace arena
{

	GameMode * GameModeFactory::createGameModeFromIndex(
		int32_t index,
		Scoreboard* m_scoreboard,
		Physics* m_physics,
		std::vector<Player>* m_players,
		GameMap* m_map,
		std::vector<const NetworkEntity*>* m_synchronizationList
	)
	{
		GameMode* m_gameMode = NULL;
		switch (index)
		{
		case 1:
			m_gameMode = new DeathMatch(1, m_scoreboard, m_physics, m_players, m_map, m_synchronizationList, 20);
			break;
		case 2:
			TeamDeathMatch* teamDeathMatch = new TeamDeathMatch(2, m_scoreboard, m_physics, m_players, m_map, m_synchronizationList, 2, false);
			teamDeathMatch->autoGroupTeams();
			m_gameMode = teamDeathMatch;
			break;
		}
		return m_gameMode;
	}

}