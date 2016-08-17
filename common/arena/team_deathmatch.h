#include "game_mode.h"
#include <algorithm>
#include "player.h"

namespace arena
{
	class TeamDeathMatch : public GameMode
	{
	public:
		TeamDeathMatch(Scoreboard* m_scoreboard, std::vector<Player>* m_players, int numTeams, bool enableAttackTeammates);
		~TeamDeathMatch();

		bool isEnd();
		bool canAttack(Gladiator* shooter, Gladiator* target);
		float calculateScore(Gladiator* shooter, Gladiator* target);
		void autoGroupTeams();

	private:
		uint8_t numTeams;
		std::vector<Player>* m_players;
		void generateMessage();
		uint8_t findTeam(PlayerScore* playerScore);
		bool enableAttackTeammates;
	};
}