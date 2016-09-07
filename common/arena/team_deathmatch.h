#include "game_mode.h"
#include <algorithm>

namespace arena
{
	class TeamDeathMatch : public GameMode
	{
	public:
		TeamDeathMatch(
			int32_t index, 
			Scoreboard* m_scoreboard, 
			Physics* m_physics, 
			std::vector<Player>* m_players, 
			GameMap* m_map, 
			std::vector<const NetworkEntity*>* m_synchronizationList, 
			int numTeams,
			bool enableAttackTeammates
		);
		~TeamDeathMatch();

		bool isEnd();
		bool canAttack(Gladiator* shooter, Gladiator* target);
		int calculateScore(Gladiator* shooter, Gladiator* target);
		void autoGroupTeams();
		void spawnPlayers();
		void spawnPlayer(Player* m_player);
		void respawnPlayers();
		void respawnPlayer(Player* m_player);

	protected:
		glm::vec2* spawnLocation(Player* m_player);

	private:
		uint8_t numTeams;
		void generateMessage();
		uint8_t findTeam(PlayerScore* playerScore);
		bool enableAttackTeammates;
	};
}