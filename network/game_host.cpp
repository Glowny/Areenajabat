#include "game_host.h"
#include <common/debug.h>
#include "server.h"
#include <common/arena/playerController.h>
#include <common/mem/packet_allocator.h>
#include <common/arena/gladiator.h>
#include <common/arena/arena_packet.h>

namespace arena
{
	GameHost::GameHost(const GameVars& vars) : m_vars(vars),
											   m_disposed(false),
											   m_endCalled(false)
	{
		// TODO: check if this is in correct place
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
		gladiator->m_grenadeWeapon  = new WeaponGrenade;
        newPlayer.m_gladiator		= gladiator;
		*gladiator->m_position =  glm::vec2(600, 200);

		uint32_t id = m_physics.addGladiator(gladiator->m_position, gladiator->m_velocity);
		
		gladiator->setPhysicsID(id);

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
		auto& players = m_players.container();


		for (auto it = players.begin(); it != players.end(); ++it)
		{
			Player& player					= *it;
			// If player is not alive, do not process input.
			if (player.m_gladiator->m_alive == false)
				continue;

			// Reset lightplatforms to solid if enought time has passed
			if ((player.m_gladiator->m_ignoreLightPlatformsTimer += (float)dt) > 0.5f)
			{
				m_physics.setGladiatorCollideLightPlatforms(player.m_gladiator->getPhysicsID(), true);
			}

			unsigned physicsID				= player.m_gladiator->getPhysicsID();
            PlayerInput& input				= player.m_playerController->m_input;

			player.m_gladiator->m_aimAngle  = player.m_playerController->aimAngle;
            
			// Check if player wants to shoot, and if weapon is able to shoot.
			// Reset shoot flag here, so that shoot messages are not missed.
			bool check = player.m_gladiator->m_weapon->checkIfCanShoot((float)dt);
			if (input.m_shootButtonDown && check)
			{
				GladiatorShoot(player.m_gladiator);
				input.m_shootButtonDown = false;
			}
			if (input.m_reloadButtonDown)
			{ 
				player.m_gladiator->m_weapon->startReload();
				player.m_gladiator->m_reloading = true;
				input.m_reloadButtonDown = false;
			}

			bool checkGrenade = player.m_gladiator->m_grenadeWeapon->checkCoolDown((float)dt);

			if (input.m_grenadeButtonDown && checkGrenade)
			{
				GrenadeShoot(player.m_gladiator);
				input.m_grenadeButtonDown = false;
			}
			player.m_gladiator->m_jumpCoolDownTimer += (float)dt;
			int32 x = 0;
			int32 y = 0;

			// TODO: Make a sensor physics object under player that detects platforms.
			if (input.m_jumpButtonDown && m_physics.checkIfGladiatorCollidesPlatform(player.m_gladiator->getPhysicsID())
				&& (player.m_gladiator->m_jumpCoolDownTimer > 0.25f))
			{
				input.m_jumpButtonDown = false;
				player.m_gladiator->m_jumpCoolDownTimer = 0;

				glm::vec2 force;
				if (x == 0)
				{
					force.y = -400.0f;
					force.x = 0;
				}
				else
				{
					force.y = -300.0f;
					force.x = x * 150.0f;
				}
				m_physics.applyImpulseToGladiator(force, physicsID);
			}

			// Do not add forces if there are none.
            if (!(input.m_leftButtonDown || input.m_rightButtonDown || input.m_upButtonDown || input.m_downButtonDown ))
				continue;
                      
          
			if (input.m_leftButtonDown)	x = -1;
			else if (input.m_rightButtonDown)	x = 1;
			if (input.m_upButtonDown) y = -1;
			else if (input.m_downButtonDown) y = 1;
 
				// reserve upbutton for ladder climb
			glm::vec2 currentVelocity = m_physics.getGladiatorVelocity(physicsID);
			float desiredVelocityX = 300.0f * (float)x;
			float velocityChangeX = desiredVelocityX - currentVelocity.x;
			// add more velocity only if climbing ladders.
			float desiredVelocityY = 0;
			float velocityChangeY = 0;

			player.m_gladiator->m_climbing = false;
			if (input.m_upButtonDown || input.m_downButtonDown)
			{
				if (input.m_downButtonDown)
				{
					player.m_gladiator->m_ignoreLightPlatformsTimer = 0.00f;
					m_physics.setGladiatorCollideLightPlatforms(player.m_gladiator->getPhysicsID(), false);
				}
				int ladderCollide = m_physics.checkIfGladiatorCollidesLadder(player.m_gladiator->getPhysicsID());
				if (ladderCollide != 0)
				{
					player.m_gladiator->m_ignoreLightPlatformsTimer = 0.40f;
					desiredVelocityY = 300.0f * (float)y;
					velocityChangeY = desiredVelocityY - currentVelocity.y;
					player.m_gladiator->m_climbing = ladderCollide;
					m_physics.setGladiatorCollideLightPlatforms(player.m_gladiator->getPhysicsID(), false);
				}
				
			}

			glm::vec2 force;
			force.y = 1500.0f * velocityChangeY * (float)dt;
			force.x = 1500.0f * velocityChangeX * (float)dt;

			m_physics.applyForceToGladiator(force, physicsID);
				
			
			
			
			// Set the inputs to zero as they are handled.
            memset(&player.m_playerController->m_input, false, sizeof(PlayerInput));
		}
	}
	void GameHost::processBulletCollisions(const float64 dt)
	{
		dt;
		std::vector<BulletCollisionEntry>& entries = m_physics.m_ContactListener.m_bulletCollisionEntries;

		if (entries.empty()) return;

		for (BulletCollisionEntry& entry : entries) 
		{
			//p_Gladiator& shooter	= entry.m_shooter;
			// TODO: make target some common type that gladiator and platform are inherited from.
			p_Bullet& bullet = entry.m_bullet;
			
			if (entry.m_target->m_type == B_Platform)
			{
				p_Platform& platform = *static_cast<p_Platform*>(entry.m_target);
				platform;

				BulletHit* hit = new BulletHit;
				hit->m_hitType = 2;
				hit->m_damageAmount = 10;
				hit->m_hitPosition = *bullet.gamePosition;
				hit->m_targetPlayerId = 0;
				hit->m_hitId = bullet.bulletId;
				b2Vec2 velocity = bullet.m_body->GetLinearVelocity();
				if (velocity.x < 0)
					hit->m_hitDirection = 0;
				else
					hit->m_hitDirection = 1;

				m_synchronizationList.push_back(hit);

			}
			else if (entry.m_target->m_type == B_Gladiator)
			{
				p_Gladiator& target = *static_cast<p_Gladiator*>(entry.m_target);
				
				Gladiator* targetGladiator = nullptr;
				
				for (auto it = m_players.begin(); it != m_players.end(); it++)
				{
					if (it->m_gladiator->getPhysicsID() == target.m_id)
					{
						targetGladiator = it->m_gladiator;
						break;
					}
				}

				if (targetGladiator == NULL)
				{ 
					// BUG: Sometimes targetGladiator is not found when there is 8+ players.
					printf("Hit was missed, searched id %d\n", target.m_id);
					return;
				}
				// if target is not alive, do not register hit.
				// TODO: set dead player to ignore bullets on physics.
				
				// Get target entity instance. Does not seem to work.
				//Gladiator* shooterGladiator = static_cast<Gladiator*>(find([&entry](NetworkEntity* const e) { return e->getPhysicsID() == entry.m_shooter.m_id; }));
				//Gladiator* targetGladiator	= static_cast<Gladiator*>(find([&target](NetworkEntity* const e) { return e->getPhysicsID() == target.m_id; }));

				if (targetGladiator->m_alive == false)
					continue;

				BulletHit* hit = new BulletHit;
				hit->m_hitType = 1;
				hit->m_hitPosition = *bullet.gamePosition;
				hit->m_hitId = bullet.bulletId;

				switch (bullet.m_bulletType)
				{
				case BulletType::GladiusBullet:
					hit->m_damageAmount = 10;
					break;
				case BulletType::ShotgunBullet:
					hit->m_damageAmount = 20;
					break;
				case BulletType::GrenadeBullet:
				{
					hit->m_damageAmount = 30;
					glm::vec2* origin = &hit->m_hitPosition;
					glm::vec2* target = targetGladiator->m_position;
					m_physics.applyGrenadeExplosionToGladiator(origin, target, targetGladiator->getPhysicsID());
					break;
				}
				default:
					break;
				}

				b2Vec2 velocity = bullet.m_body->GetLinearVelocity();
				if (velocity.x < 0)
					hit->m_hitDirection = 0;
				else
					hit->m_hitDirection = 1;
				hit->m_targetPlayerId = target.m_id;

				targetGladiator->m_hitpoints -= hit->m_damageAmount;
				if (targetGladiator->m_hitpoints <= 0)
				{ 
					// Search for shooter.
					unsigned shooterPlayerId = 666;
					for (auto it = m_players.begin(); it != m_players.end(); it++)
					{
						if (it->m_gladiator->getPhysicsID() == entry.m_shooter.m_id)
						{
							shooterPlayerId = it->m_gladiator->m_ownerId;
							break;
						}
					}
					// assert that shooter is found.
					assert(shooterPlayerId != 666);

					targetGladiator->m_alive = false;
					PlayerScore &targetScore = m_scoreBoard->getPlayerScore(targetGladiator->m_ownerId);
					PlayerScore &shooterScore = m_scoreBoard->getPlayerScore(shooterPlayerId);

					if (targetScore.m_tickets > 0)
						targetScore.m_tickets--;
					shooterScore.m_kills++;
					shooterScore.m_score += 10;
					m_synchronizationList.push_back(m_scoreBoard);
					
				}
				m_synchronizationList.push_back(hit);

				// Sync.
				m_synchronizationList.push_back(targetGladiator);
				
				// TODO: Do removal here and properly
				
			
				delete entry.m_target;
			}
			for (unsigned i = 0; i < m_debugBullets.size(); i++)
				if (m_debugBullets[i].m_bullet->m_bulletId == bullet.bulletId)
				{
					m_physics.removeBullet(m_debugBullets[i].m_bullet->m_bulletId);

					delete m_debugBullets[i].m_bullet;

					m_debugBullets.erase(m_debugBullets.begin() + i);
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

	void GameHost::GrenadeShoot(Gladiator* gladiator) {
		Bullet* bullet = gladiator->pitch();
		bullet->m_shooterId = gladiator->getPhysicsID();
		gladiator->getPhysicsID();
		bullet->m_bulletId = m_physics.addGrenade(bullet->m_position, bullet->m_impulse, gladiator->getPhysicsID());

		m_synchronizationList.push_back(bullet);
		DebugBullet dBullet;
		dBullet.lifeTime = 13;
		dBullet.m_bullet = bullet;
		m_debugBullets.push_back(dBullet);
	}

	void GameHost::GladiatorShoot(Gladiator* gladiator)
	{
		std::vector<Bullet*> bullets = gladiator->shoot();
		
		for(uint32 i = 0; i < bullets.size(); i++)
		{ 
			bullets[i]->m_bulletId = m_physics.addBullet(bullets[i]->m_position, bullets[i]->m_impulse, gladiator->getPhysicsID());
			
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


	GameHost::~GameHost()
	{
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

			//TODO: Move this check to slave_server, and start round from there.
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

            loadMap("assets/coordinatesRawData.dat");
			for (uint32 i = 0; i < m_map.m_platformVector.size(); i++)
			{
				m_physics.createPlatform(m_map.m_platformVector[i].vertices, m_map.m_platformVector[i].type);
			}

			for (auto it = m_players.begin(); it != m_players.end(); it++)
			{
				Player* player = &*it;
				uint32 physicsID = player->m_gladiator->getPhysicsID();
				m_physics.setGladiatorPosition(physicsID, m_map.m_playerSpawnLocations[physicsID]);
			}
			addScoreBoard();
	
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
			
			if ((m_physics.updateTimer += dt) >= PHYSICS_TIMESTEP)
			{
				// Update physics
				m_physics.update(m_physics.updateTimer);
				//TODO: uncomment check when confirmed working
				//if(shouldProcessPlayerInput())
				applyPlayerInputs(m_physics.updateTimer);
				processBulletCollisions(m_physics.updateTimer);
				// get data from gladiators.
				for (Player& player : players())
				{
					if (player.m_gladiator->m_alive == false)
					{ 
						if (player.m_gladiator->checkRespawn(m_physics.updateTimer)) 
						{
					
							player.m_gladiator->m_alive = true;
							player.m_gladiator->m_hitpoints = 100;
							m_physics.setGladiatorPosition(player.m_gladiator->getPhysicsID(), glm::vec2(1600,200));
							m_physics.applyImpulseToGladiator(glm::vec2(1, 1), player.m_gladiator->getPhysicsID());
							printf("Respawned player %d\n", player.m_gladiator->getPhysicsID());
							// HAX, USE EVENTHANDLER
							NetworkEntity* entity = new NetworkEntity(NetworkEntityType::RespawnPlayer, 0);
							entity->setPhysicsID(player.m_gladiator->getPhysicsID());
							m_synchronizationList.push_back(entity);
							// No delete because this needs to be remade.
						}
					}
					 // update position because of gravity - dont update too much
					 m_synchronizationList.push_back(player.m_gladiator);
				}

				for (unsigned i = 0; i < m_debugBullets.size(); ++i)
				{
					if ((m_debugBullets[i].lifeTime += m_physics.updateTimer) < 20.0f)
					{ 
						
						m_synchronizationList.push_back(m_debugBullets[i].m_bullet);
					}
					else
					{
						m_physics.removeBullet(m_debugBullets[i].m_bullet->m_bulletId);

						delete m_debugBullets[i].m_bullet;
						
						m_debugBullets.erase(m_debugBullets.begin() + i);
					}
					if (m_debugBullets[i].m_bullet->m_type == GrenadeBullet)
					{
						Grenade* grenade = static_cast<Grenade*>(m_debugBullets[i].m_bullet);
						grenade->m_timer += m_physics.updateTimer;
						if (grenade->m_timer > grenade->m_endTime)
						{
							//printf("Delete explosion and grenade\n");
							// Remove explosion and grenade created below.
							m_physics.removeBullet(grenade->m_explosionId);
							m_physics.removeBullet(grenade->m_bulletId);
							delete m_debugBullets[i].m_bullet;
							m_debugBullets.erase(m_debugBullets.begin() + i);
						}
				
						if (grenade->m_timer> grenade->m_explosionTime && !grenade->isExplosion)
						{
							//                         printf("Explosion\n");
							// Create explosion and save id on m_explosionId
							grenade->isExplosion = true;
							grenade->m_explosionId = m_physics.addExplosion(grenade->m_position, 200, grenade->m_shooterId);
						}
					}
					
				}
				m_physics.updateTimer = 0;
			}
		}
	}

	void GameHost::addScoreBoard()
	{
		Scoreboard* board = new Scoreboard;
		for (auto it = m_players.begin(); it != m_players.end(); it++)
		{
			Player* player = &*it;
			PlayerScore score;
			score.m_kills = 0;
			score.m_score = 0;
			//TODO: get the amount of tickets from initilization file.
			score.m_tickets = 10;
			score.m_playerID = player->m_clientIndex;
			board->m_playerScoreVector.push_back(score);
		}
		m_scoreBoard = board;
		registerEntity(board);
		m_synchronizationList.push_back(m_scoreBoard);
	}
	void GameHost::resetScoreBoard()
	{
		assert(m_scoreBoard != nullptr);
		for (auto it = m_scoreBoard->m_playerScoreVector.begin(); it != m_scoreBoard->m_playerScoreVector.end(); it++)
		{
			it->m_kills = 0;
			it->m_score = 0;
			//TODO: get the amount of tickets from initilization file
			it->m_tickets = 1;
		}
	}
	void GameHost::emptyScoreBoard()
	{
		assert(m_scoreBoard != nullptr);
		m_scoreBoard->m_playerScoreVector.clear();
	}
	Scoreboard& GameHost::getScoreBoard()
	{
		assert(m_scoreBoard != nullptr);
		return (*m_scoreBoard);
	}
}