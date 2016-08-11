#include <string>
#include "scoreboard.h"

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

	protected:
		std::string endMessage;
		Scoreboard* m_scoreboard;


	};
}