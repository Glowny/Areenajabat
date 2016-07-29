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
		m_gameData.m_state = GameState::RoundRunning;
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

        Player newPlayer;
		newPlayer.m_clientIndex		 = clientIndex;
		newPlayer.m_playerController = new PlayerController();

        Gladiator* gladiator		= new Gladiator;
		gladiator->m_ownerId		= newPlayer.m_clientIndex;
		gladiator->m_weapon			= new WeaponGladius;
        newPlayer.m_gladiator		= gladiator;
		*gladiator->m_position =  glm::vec2(600, 200);

		uint32_t id = m_physics.addGladiator(gladiator->m_position);
		gladiator->m_physicsId = id;

        m_players.add(newPlayer);

		registerEntity(&m_players.back());
	}
	void GameHost::unregisterPlayer(const uint32 clientIndex)
	{
		ARENA_ASSERT(isStateValid(), "State isn't valid");
		
		for (auto it = m_players.begin(); it != m_players.end(); it++)
		{
            Player* player = &*it;

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

	void GameHost::applyPlayerInputs(const float64 dt)
	{
		// TODO: add jump and dont let player decide amount of force applied!
		auto& players = m_players.container();


		for (auto it = players.begin(); it != players.end(); ++it)
		{
			Player& player					= *it;
			// If player is not alive, do not process input.
			if (player.m_gladiator->m_alive == false)
				continue;

			unsigned physicsId				= player.m_gladiator->m_physicsId;
            PlayerInput& input				= player.m_playerController->m_input;

			player.m_gladiator->m_aimAngle  = player.m_playerController->aimAngle;
            
			// Check if player wants to shoot, and if weapon is able to shoot.
			// Reset shoot flag here, so that shoot messages are not missed.
			bool check = player.m_gladiator->m_weapon->checkCoolDown((float)dt);
			if (input.m_shootButtonDown && check)
			{
				GladiatorShoot(player.m_gladiator);
				input.m_shootButtonDown = false;
			}

			
			// Do not add forces if there are none.
            if (!(input.m_leftButtonDown || input.m_rightButtonDown || input.m_upButtonDown)) continue;
                      
            int32 x = 0;
			int32 y = 0;
            
			if		(input.m_leftButtonDown)	x = -1;
            else if (input.m_rightButtonDown)	x = 1;

            if (input.m_upButtonDown) y = 10;

            glm::ivec2 moveDirection(x, y);
      
			glm::vec2 currentVelocity	= m_physics.getGladiatorVelocity(physicsId);

			if (int32(currentVelocity.x) < 200 && int32(currentVelocity.x) > -200)
			{
				glm::vec2 force;

				force.y = moveDirection.y * -30000.0f * (float32)dt;
				force.x = moveDirection.x * 150000.0f * (float32)dt;

				m_physics.applyForceToGladiator(force, physicsId);
				
			}

			
			// Set the inputs to zero as they are handled.
            memset(&player.m_playerController->m_input, false, sizeof(PlayerInput));
		}
	}
	void GameHost::processBulletCollisions(const float64 dt)
	{
		std::vector<BulletCollisionEntry>& entries = m_physics.m_ContactListener.m_bulletCollisionEntries;

		if (entries.empty()) return;

		for (BulletCollisionEntry& entry : entries) 
		{
			//p_Gladiator& shooter	= entry.m_shooter;
			// TODO: wall hits are also registered here, do some checks.
			p_Gladiator& target		= entry.m_target;
			p_Bullet& bullet		= entry.m_bullet;
			dt;
			
			if (target.m_id > 10)
				continue;
			// Get target entity instance. Does not seem to work.
			//Gladiator* shooterGladiator = static_cast<Gladiator*>(find([&shooter](NetworkEntity* const e) { return e->getPhysicsID() == shooter.m_id; }));
			//Gladiator* targetGladiator	= static_cast<Gladiator*>(find([&target](NetworkEntity* const e) { return e->getPhysicsID() == target.m_id; }));

		
			BulletHit* hit = new BulletHit;
			hit->m_damageAmount = 10;
			hit->m_hitPosition = bullet.hitPosition - m_physics.getGladiatorPosition(target.m_id);

			b2Vec2 velocity = bullet.m_body->GetLinearVelocity();
			if (velocity.x < 0)
				hit->m_hitDirection = 0;
			else
				hit->m_hitDirection = 1;
			hit->m_targetPlayerId = target.m_id;
			Gladiator* targetGladiator = nullptr;
			std::vector<Player> temp = players();

			for (unsigned i = 0; i < players().size(); i++)
			{
				if (hit->m_targetPlayerId == players()[i].m_gladiator->m_physicsId)
				{
					targetGladiator = players()[i].m_gladiator;
					continue;
				}
			}

			targetGladiator->m_hitpoints -= hit->m_damageAmount;
			if (targetGladiator->m_hitpoints < 0)
			{ 
				targetGladiator->m_alive = false;
				printf("Killed player %d\n", targetGladiator->getPhysicsID());
			}
			m_synchronizationList.push_back(hit);

			// Sync.
			m_synchronizationList.push_back(targetGladiator);
			
			// TODO: Do removal here and properly
			for (unsigned i = 0; i < m_debugBullets.size(); i++)
			{ 
				if (m_debugBullets[i].m_bullet->m_bulletId == bullet.bulletId)
					m_debugBullets[i].lifeTime = 10.0f;
			}
		}

		entries.clear();
	}

	void GameHost::loadMap(const char* const mapName)
	{
		m_map.loadMapFromFile(mapName);
		m_synchronizationList.push_back(&m_map);
	}

	NetworkEntity* const GameHost::find(Predicate<NetworkEntity* const> pred)
	{
		for (NetworkEntity* const e : m_entities) if (pred(e)) return e;

		return nullptr;
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

	void GameHost::processInput(const uint64 clientIndex, const PlayerInput& input, const float32 aimAngle)
	{
		// TODO: do proper check.
		//if (!shouldProcessPlayerInput()) return;

		Player* const player = m_players.find([&clientIndex](const Player* const p) { return p->m_clientIndex == clientIndex; });

		if (player == nullptr) return;
		
		// Do stuff with this on physics update.
        player->m_playerController->m_input = input;
		player->m_playerController->aimAngle = aimAngle;
	}
	void GameHost::GladiatorShoot(Gladiator* gladiator)
	{
		// TODO: do proper check.
		//if (!shouldProcessPlayerInput()) return;

		// Note: Bullets are extremely short-lived. They are not updated to players, and only bullet hits are registered from physics.
		// Bullets should be deleted after synchronization, should slave delete them?
		// No, host owns them. Host should delete them.
		std::vector<Bullet*> bullets = gladiator->createBullets();
		
		for(uint32 i = 0; i < bullets.size(); i++)
		{ 
			bullets[i]->m_bulletId = m_physics.addBullet(bullets[i]->m_position, bullets[i]->m_impulse, gladiator->m_physicsId);
			
			m_synchronizationList.push_back(bullets[i]);
			DebugBullet dBullet;
			dBullet.lifeTime = 0;
			dBullet.m_bullet = bullets[i];
			m_debugBullets.push_back(dBullet);
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

			for (uint32 i = 0; i < m_map.m_platformVector.size(); i++)
			{
				m_physics.createPlatform(m_map.m_platformVector[i].vertices, m_map.m_platformVector[i].type);
			}
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
			
			if ((m_physics.updateTimer += float32(dt)) > TIMESTEP)
			{
				//TODO: uncomment check when confirmed working
				//if(shouldProcessPlayerInput())
				applyPlayerInputs(m_physics.updateTimer);

				// Update physics
				m_physics.update(m_physics.updateTimer);
				processBulletCollisions(m_physics.updateTimer);
				// get data from gladiators.
				for (Player& player : players())
				{
					if (player.m_gladiator->m_alive == false)
					{ 
						if (player.m_gladiator->checkRespawn(m_physics.updateTimer)) 
						{
							player.m_gladiator->m_alive = true;
							player.m_gladiator->m_hitpoints = 100.0f;
							m_physics.setGladiatorPosition(player.m_gladiator->getPhysicsID(), glm::vec2(1600,200));
							printf("Respawned player %d\n", player.m_gladiator->getPhysicsID());
						}
					}
					 // update position because of gravity - dont update too much
					 m_synchronizationList.push_back(player.m_gladiator);
				}

				for (unsigned i = 0; i < m_debugBullets.size(); ++i)
				{
					if ((m_debugBullets[i].lifeTime += m_physics.updateTimer) < 0.9f)
					{ 
						
						m_synchronizationList.push_back(m_debugBullets[i].m_bullet);
					}
					else
					{
						m_physics.removeBullet(m_debugBullets[i].m_bullet->m_bulletId);

						delete m_debugBullets[i].m_bullet;
						
						m_debugBullets.erase(m_debugBullets.begin() + i);
					}
					
				}
				m_physics.updateTimer = 0;
			}
		}
	}

	GameHost::~GameHost()
	{
	}
}