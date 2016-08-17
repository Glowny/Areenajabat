#include "game_mode.h"

namespace arena
{
	class DeathMatch : public GameMode
	{
	public:
		DeathMatch(Scoreboard* m_scoreboard, int maxScore);
		~DeathMatch();

		bool isEnd();
		bool canAttack(Gladiator* shooter, Gladiator* target);
		float calculateScore(Gladiator* shooter, Gladiator* target);

	private:
		int maxScore;
		void generateMessage();
	};
}