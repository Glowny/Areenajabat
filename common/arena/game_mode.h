#pragma once

#include <string>
#include "scoreboard.h"
#include "gladiator.h"
#include <sstream>

namespace arena
{
	class GameMode
	{
	public:
		GameMode(int32_t index, Scoreboard* m_scoreboard);
		virtual ~GameMode() = 0;

		virtual bool isEnd() = 0;
		virtual bool canAttack(Gladiator* shooter, Gladiator* target) = 0;
		virtual float calculateScore(Gladiator* shooter, Gladiator* target) = 0;
		//void setEndMessage(std::string endMessage);
		std::string getEndMessage();
		bool updateEndTimer(float dt);
		void resetEndTimer();

		int32_t index;

	protected:
		std::string endMessage;
		Scoreboard* m_scoreboard;
		float waitingEndTime;
		float timer;
		bool msgChecker;


	};
}