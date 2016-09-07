#include "game_mode.h"
#include <random>
#include <list>

namespace arena
{
	class DeathMatch : public GameMode
	{
	public:
		DeathMatch(
			int32_t index,
			Scoreboard* m_scoreboard,
			Physics* m_physics,
			std::vector<Player>* m_players,
			GameMap* m_map,
			std::vector<const NetworkEntity*>* m_synchronizationList,
			int maxScore
		);
		~DeathMatch();

		bool isEnd();
		bool canAttack(Gladiator* shooter, Gladiator* target);
		int calculateScore(Gladiator* shooter, Gladiator* target);
		void spawnPlayers();
		void spawnPlayer(Player* m_player);
		void respawnPlayers();
		void respawnPlayer(Player* m_player);

	protected:
		glm::vec2* spawnLocation(Player* m_player);

	private:
		int maxScore;
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution;
		void generateMessage();
		std::list<int> locationHistory;

	};
}