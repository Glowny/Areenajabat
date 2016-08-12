#include <string>
#include "scoreboard.h"
#include <sstream>

namespace arena
{
	class GameMode
	{
	public:
		GameMode(Scoreboard* m_scoreboard);
		virtual ~GameMode() = 0;

		virtual bool isEnd() = 0;
		//void setEndMessage(std::string endMessage);
		std::string getEndMessage();
		bool updateEndTimer(float dt);
		void resetEndTimer();
	protected:
		std::string endMessage;
		Scoreboard* m_scoreboard;
		float waitingEndTime;
		float timer;


	};
}