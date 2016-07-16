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
		if (m_gameData.m_timeout) return;

		m_gameData.m_timeoutElapsed = 0;
		m_gameData.m_timeout = true;

		e_timeoutStart();
	}
	void GameHost::timeoutEnd()
	{
		if (!m_gameData.m_timeout) return;

		m_gameData.m_timeoutElapsed = 0;
		m_gameData.m_timeout = false;

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

		m_entities.add(newPlayer);
	}
	void GameHost::unregisterPlayer(const uint32 clientIndex)
	{
		ARENA_ASSERT(isStateValid(), "State isn't valid");
		
		for (auto it = m_players.begin(); it != m_players.end(); it++)
		{
			const Player* player = &*it;

			if (player->m_clientIndex == clientIndex) 
			{
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
		auto& players = m_players.container();

		for (auto it = players.begin(); it != players.end(); ++it)
		{
			Player& player = *it;

			unsigned physicsId			= player.m_gladiator->m_physicsId;
			glm::ivec2 moveDirection	= player.m_playerController->m_movementDirection;
			glm::vec2 currentVelocity	= m_physics.getGladiatorVelocity(physicsId);

			if (int32(currentVelocity.x) < 250 && int32(currentVelocity.x) > -250)
			{
				glm::vec2 force;

				force.x = moveDirection.x * 1500.0f;

				m_physics.applyForceToGladiator(force, physicsId);
			}
		}
	}

	void GameHost::loadMap(const char* const mapName)
	{
		m_map.loadMapFromFile(mapName);
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

	void GameHost::processInput(const uint64 clientIndex, const float32 x, const float32 y)
	{
		(void)clientIndex;
		(void)x;
		(void)y;
	}
	void GameHost::processShooting(const uint64 clientIndex, const bool flags, const float32 angle)
	{
		(void)clientIndex;
		(void)flags;
		(void)angle;
	}
	
	void GameHost::clearPackets()
	{
		PacketAllocator& allocator = PacketAllocator::instance();

		for (Packet* const packet : m_outPackets)
		{
			allocator.deallocate(packet);
		}

		m_outPackets.clear();
	}
	const std::vector<Packet*>& GameHost::getResults()
	{
		return m_outPackets.container();
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

		if (m_players.size() >= m_vars.m_gm_players_required)
		{
			m_gameData.m_gameRunning = true;
			
			m_sessionData.m_sessionElapsed = 0;
			m_gameData.m_timeoutElapsed = 0;
			m_gameData.m_gameElapsed = 0;
			m_gameData.m_roundTimeElapsed = 0;
			m_gameData.m_roundsCount = 0;
			m_gameData.m_roundFreezeTimeElapsed = 0;

			e_gameStart();
		}
	}
	void GameHost::gameTick(const uint64 dt)
	{
		if (!m_gameData.m_gameRunning) return;
		
		if (m_gameData.m_timeout)
		{
			if (m_gameData.m_timeoutElapsed == 0) e_timeoutStart();

			m_gameData.m_timeoutElapsed += dt;
			m_gameData.m_timeout = m_gameData.m_timeoutElapsed < 60;

			if (!m_gameData.m_timeout)
			{
				m_gameData.m_timeoutElapsed = 0;
				
				e_timeoutEnd();
			}

			return;
		}

		if (!m_gameData.m_roundRunning)
		{
			if (m_gameData.m_roundFreezeTimeElapsed >= m_vars.m_gm_round_freeze_time)
			{
				m_gameData.m_roundFreezeTimeElapsed = 0;
				m_gameData.m_roundRunning = true;

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
				m_gameData.m_roundRunning = false;

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
		if (m_gameData.m_timeout)
		{
			// Do not apply any player input updates.
		}
		else if (m_gameData.m_roundRunning)
		{
			// Do normal updates.
		}

		// Update.
		if ((m_physics.updateTimer += dt) > TIMESTEP) m_physics.update();
	}

	GameHost::~GameHost()
	{
	}
}