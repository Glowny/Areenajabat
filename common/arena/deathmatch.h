#include "game_mode.h"

namespace arena
{
	class DeathMatch : public GameMode
	{
	public:
		DeathMatch(Scoreboard* m_scoreboard, int maxScore, int maxKills);
		~DeathMatch();

		bool isEnd();

	private:
		int maxScore;
		int maxKills;
	};
}