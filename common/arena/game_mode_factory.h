#pragma once

#include "deathmatch.h"
#include "team_deathmatch.h"

namespace arena
{
	class GameModeFactory
	{
	public:
		static GameMode* createGameModeFromIndex(int32_t index, Scoreboard* m_scoreboard, std::vector<Player>* m_players);

	};
}