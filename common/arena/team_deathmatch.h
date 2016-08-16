#include "game_mode.h"
#include <algorithm>
#include "player.h"
#include <map>

namespace arena
{
	class TeamDeathMatch : public GameMode
	{
	public:
		TeamDeathMatch(Scoreboard* m_scoreboard, std::vector<Player>* m_players, int numTeams);
		~TeamDeathMatch();

		bool isEnd();
		void autoGroupTeams();

	private:
		uint8_t numTeams;
		std::vector<Player>* m_players;
		void generateMessage();
		uint8_t findTeam(PlayerScore* playerScore);
	};
}