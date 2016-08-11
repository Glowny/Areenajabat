#include "game_mode.h"

namespace arena
{
	GameMode::GameMode(Scoreboard* m_scoreboard)
	{
		GameMode::m_scoreboard = m_scoreboard;
		GameMode::endMessage = "TEST";
	}
	GameMode::~GameMode()
	{
		printf("Deleted GameMode\n");
	}

	std::string GameMode::getEndMessage()
	{
		return GameMode::endMessage;
	}


}