#include "game_mode.h"

namespace arena
{
	GameMode::GameMode(Scoreboard* m_scoreboard)
	{
		GameMode::m_scoreboard = m_scoreboard;
		waitingEndTime = 200000.0f;
		GameMode::timer = 0;
	}
	GameMode::~GameMode()
	{
		printf("Deleted GameMode\n");
	}

	std::string GameMode::getEndMessage()
	{
		return GameMode::endMessage;
	}

	bool GameMode::updateEndTimer(float dt) {
		GameMode::timer += dt;
		return (GameMode::timer > GameMode::waitingEndTime);
	}

	void GameMode::resetEndTimer(){
		GameMode::timer = 0;
	}

}