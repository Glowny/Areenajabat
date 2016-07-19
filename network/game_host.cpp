#include "game_host.h"
#include <common\debug.h>
#include "server.h"
#include <common\arena\playerController.h>
#include <common\mem\packet_allocator.h>
#include <common\arena\gladiator.h>

namespace arena
{
	GameHost::GameHost(const GameVars& vars) : m_vars(vars),
											   m_disposed(false),
											   m_endCalled(false)
	{
		m_physics = physics();
	}

	void GameHost::startSession() 
	{
		if (m_sessionData.m_sessionRunning) return;

		e_sessionStart();

		m_sessionData.m_sessionRunning = true;
	}
	void GameHost::endSession()
	{
		if (!m_sessionData.m_sessionRunning) return;

		e_sessionEnd();

		m_sessionData.m_sessionRunning = false;
	}

	void GameHost::endGame()
	{
		if (m_endCalled) return;

		// End the game.

		e_gameEnd();

		m_endCalled = true;
	}
	void GameHost::dispose()
	{
		if (m_disposed) return;

		// Dispose of resources.
		// Last call.

		m_disposed = true;
	}

	void GameHost::timeOutBegin()
	{
		if (m_gameData.m_state == GameState::Timeout) return;

		m_gameData.m_timeoutElapsed = 0;
		m_gameData.m_state = GameState::Running;

		e_timeoutStart();
	}
	void GameHost::timeoutEnd()
	{
		if (!(m_gameData.m_state == GameState::Timeout)) return;

		m_gameData.m_timeoutElapsed = 0;
		m_gameData.m_state = GameState::Running;

		e_timeoutEnd();
	}

	void GameHost::forceShutdown()
	{
		endSession();
		endGame();
		dispose();
	}

	bool GameHost::isStateValid() const
	{
		return !m_disposed && !m_endCalled;
	}

	void GameHost::tick(const float64 dt)
	{
		const uint64 uidt = uint64(dt);

		if (m_sessionData.m_sessionRunning) sessionTick(uidt);
		
		if (m_gameData.m_gameRunning)
		{
			gameTick(uidt);
			
			worldTick(dt);
		}
	}

	void GameHost::registerPlayer(const uint32 clientIndex)
	{
		ARENA_ASSERT(isStateValid(), "State isn't valid");
		
		DEBUG_PRINT("new player registered...");

		m_players.add(Player());

		Player* const newPlayer			= &m_players.back();
		newPlayer->m_clientIndex		= clientIndex;
		newPlayer->m_playerController	= new PlayerController();

        Gladiator* gladiator = new Gladiator;

        gladiator->m_physicsId = m_physics.addGladiator(m_map.m_playerSpawnLocations[clientIndex]);
        gladiator->m_weapon = new WeaponGladius;
        newPlayer->m_gladiator = gladiator;

		registerEntity(newPlayer);
	}
	void GameHost::unregisterPlayer(const uint32 clientIndex)
	{
		ARENA_ASSERT(isStateValid(), "State isn't valid");
		
		for (auto it = m_players.begin(); it != m_players.end(); it++)
		{
			Player* player = const_cast<Player*>(&*it);

			if (player->m_clientIndex == clientIndex) 
			{
				// TODO: unregister all entitites that the
				//		 player owns as well.
				unregisterEntity(player);
				
				m_players.remove(*it);
			
				return;
			}
		}
	}

	void GameHost::registerEntity(NetworkEntity* entity)
	{
		if (m_entities.contains(entity)) return;

		m_entities.add(entity);
	}
	void GameHost::unregisterEntity(NetworkEntity* entity)
	{
		if (!m_entities.contains(entity)) return;

		m_entities.remove(entity);
	}

	void GameHost::applyPlayerInputs()
	{
		// TODO: add jump and dont let player decide amount of force applied!
		auto& players = m_players.container();

		for (auto it = players.begin(); it != players.end(); ++it)
		{
			Player& player = *it;
			unsigned physicsId			= player.m_gladiator->m_physicsId;
			glm::ivec2 moveDirection	= player.m_playerController->m_movementDirection;
			// Do not add forces if there are none.
			if (moveDirection.x == 0 && moveDirection.y == 0)
				continue;
			glm::vec2 currentVelocity	= m_physics.getGladiatorVelocity(physicsId);


			if (int32(currentVelocity.x) < 250 && int32(currentVelocity.x) > -250)
			{
				glm::vec2 force;

				// TODO: Make better, done because network is not working correctly fix later
				if (moveDirection.x == 2)
					moveDirection.x = -1;

				force.x = moveDirection.x * 1500.0f;

				m_physics.applyForceToGladiator(force, physicsId);
			
				m_synchronizationList.push_back(player.m_gladiator);
			}
			// Set the inputs to zero as they are handled.
			player.m_playerController->m_movementDirection.x = 0;
			player.m_playerController->m_movementDirection.y = 0;
		}
	}

	void GameHost::loadMap(const char* const mapName)
	{
		m_map.loadMapFromFile(mapName);
		m_synchronizationList.push_back(&m_map);
	}

	std::vector<Player>& GameHost::players()
	{
		return m_players.container();
	}
	GameMap& GameHost::map()
	{
		return m_map;
	}
	Physics& GameHost::physics()
	{
		return m_physics;
	}

	void GameHost::getSynchronizationList(std::vector<const NetworkEntity*>& outSynchronizationList)
	{
		 outSynchronizationList = m_synchronizationList;
		 m_synchronizationList.clear();
	}

	void GameHost::processInput(const uint64 clientIndex, const float32 x, const float32 y)
	{
		// TODO: do proper check.
		//if (!shouldProcessPlayerInput()) return;

		Player* const player = m_players.find([&clientIndex](const Player* const p) { return p->m_clientIndex == clientIndex; });

		if (player == nullptr) return;


		// Apply force only when physics are also updated.
		player->m_playerController->m_movementDirection.x = x;
		player->m_playerController->m_movementDirection.y = y;
	}
	void GameHost::processShooting(const uint64 clientIndex, const bool flags, const float32 angle)
	{
		if (!shouldProcessPlayerInput()) return;

		(void)clientIndex;
		(void)flags;
		(void)angle;

		Player* const player = m_players.find([&clientIndex](const Player* const p) { return p->m_clientIndex == clientIndex; });

		if (player == nullptr) return;
		
		// Note: Bullets are extremely short-lived. They are not updated to players, and only bullet hits are registered from physics.
		// Bullets should be deleted after synchronization, should slave delete them?

		std::vector<Bullet*> bullets = player->m_gladiator->m_weapon->createBullets(angle, player->m_gladiator->m_position);
		
		for(unsigned i = 0; i < bullets.size(); i++)
		{ 
			m_physics.addBullet(bullets[i]->m_position, bullets[i]->m_impulse, player->m_gladiator->m_physicsId);
			m_synchronizationList.push_back(bullets[i]);
		}

		
	}
	
	bool GameHost::shouldProcessPlayerInput() const
	{
		// Round freeze.
		if (m_gameData.m_roundsCount >= m_vars.m_gm_rounds_count)			return false;
		// Game and session must be running.
		if (!m_gameData.m_gameRunning || !m_sessionData.m_sessionRunning)	return false;
		// Do not process input at freezetime.
		if (m_gameData.m_state == GameState::Freezetime)					return false;

		return true;
	}

	void GameHost::sessionTick(const uint64 dt)
	{
		m_sessionData.m_sessionElapsed += dt;

		if (!m_gameData.m_gameRunning)
		{
			if (m_sessionData.m_sessionElapsed > m_vars.m_gm_player_wait_time)
			{
				if (m_vars.m_gm_shutdown_after_wait)
				{
					// Not enough players connected, shutdown.
					e_sessionRestart();
				}
				else 
				{
					// Restart wait time.
					m_sessionData.m_sessionElapsed = 0;
				}
			}
		}

		if (m_players.size() >= m_vars.m_gm_players_required && !m_gameData.m_gameRunning)
		{
			m_gameData.m_gameRunning = true;
			
			m_sessionData.m_sessionElapsed = 0;
			m_gameData.m_timeoutElapsed = 0;
			m_gameData.m_gameElapsed = 0;
			m_gameData.m_roundTimeElapsed = 0;
			m_gameData.m_roundsCount = 0;
			m_gameData.m_roundFreezeTimeElapsed = 0;

			e_gameStart();
            loadMap("coordinatesRawData.dat");
		}
	}
	void GameHost::gameTick(const uint64 dt)
	{
		if (!m_gameData.m_gameRunning) return;
		
		// TODO: add freezetime.

		if (m_gameData.m_state == GameState::Timeout)
		{
			if (m_gameData.m_timeoutElapsed == 0) e_timeoutStart();

			m_gameData.m_timeoutElapsed += dt;
			m_gameData.m_state = m_gameData.m_timeoutElapsed < 60 ? GameState::Timeout : GameState::Running;

			if (m_gameData.m_state != GameState::Timeout)
			{
				m_gameData.m_timeoutElapsed = 0;
				
				e_timeoutEnd();
			}

			return;
		}

		if (m_gameData.m_state != GameState::RoundRunning)
		{
			if (m_gameData.m_roundFreezeTimeElapsed >= m_vars.m_gm_round_freeze_time)
			{
				m_gameData.m_roundFreezeTimeElapsed = 0;
				m_gameData.m_state = GameState::RoundRunning;

				// Round start.
				e_roundStart();
			}
			else
			{
				m_gameData.m_roundFreezeTimeElapsed += dt;
			}
		}
		else
		{
			if (m_gameData.m_roundTimeElapsed >= m_vars.m_gm_round_duration)
			{
				m_gameData.m_roundTimeElapsed = 0;
				m_gameData.m_state = GameState::Running;

				e_roundEnd();

				m_gameData.m_roundsCount++;

				if (m_gameData.m_roundsCount >= m_vars.m_gm_rounds_count)
				{
					e_roundLimit();
				}
			}
			else
			{
				m_gameData.m_roundTimeElapsed += dt;
			}
		}

		m_gameData.m_gameElapsed += dt;
	}

	void GameHost::worldTick(const float64 dt)
	{
		if (m_gameData.m_state == GameState::Timeout)
		{
			// Do not apply any player input updates.
		}
		else if (m_gameData.m_state == GameState::RoundRunning)
		{
			// Do normal updates.
			
			if ((m_physics.updateTimer += dt) > TIMESTEP)
			{
				//TODO: uncomment check when confirmed working
				//if(shouldProcessPlayerInput())
					applyPlayerInputs();
				// Update physics
				m_physics.update();

				// get data from gladiators.
				for (Player& player : players())
				{
					player.m_gladiator->m_position = m_physics.getGladiatorPosition(player.m_gladiator->m_physicsId);
					// update position because gravity
					m_synchronizationList.push_back(player.m_gladiator);
				}
				
			}
		}

		
	}

	GameHost::~GameHost()
	{
	}
}