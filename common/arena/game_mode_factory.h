#pragma once

#include "deathmatch.h"
#include "team_deathmatch.h"

namespace arena
{
	class GameModeFactory
	{
	public:
		static GameMode* createGameModeFromIndex(
			int32_t index,
			Scoreboard* m_scoreboard,
			Physics* m_physics,
			std::vector<Player>* m_players,
			GameMap* m_map,
			std::vector<const NetworkEntity*>* m_synchronizationList
		);

	};
}