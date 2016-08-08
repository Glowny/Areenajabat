#include "sandbox_scene.h"
#include "../app.h"
#include "../game_time.h"

#include "../ecs/projectile.h"
#include "../ecs/timer.h"
#include "../ecs/movement.h"
#include "../ecs/transform.h"
#include "../ecs/animator.h"
#include "../ecs/sprite_renderer.h"
#include "../ecs/entity.h"
#include "../ecs/entity_builder.h"
#include "../ecs/managers/sprite_manager.h"
#include "../ecs/managers/animator_manager.h"

#include "../res/resource_manager.h"
#include "../res/texture_resource.h"
#include "../res/spriter_resource.h"

#include "../net/network_client.h"

#include "../graphics/character_animator.h"
#include "../graphics/spritebatch.h"

#include "../utils/color.h"
#include "../utils/math.h"

#include "../input/event.h"

#include <common/arena/gladiator.h>
#include <common/packet.h>
#include <bx/fpumath.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#ifdef _DEBUG

#   include "../rtti/rtti_define.h"
#endif


namespace arena
{
	glm::vec2 radToVec(float r)
	{
		return glm::vec2(cos(r), sin(r));
	}


	void loadServerInfoFromFile(std::string& ip, uint16_t& port)
	{
		std::ifstream file("assets/ServerInfo.txt");
		if (file.is_open())
		{
			getline(file, ip);
			std::string tempString;
			getline(file, tempString);
			port = std::stoul(tempString, nullptr, 0);
			
		}
	}

	void GladiatorDrawData::destroy()
	{
		m_entity->destroy();
		delete m_gladiator;
	}

	SandboxScene* sandbox;
	static NetworkClient* s_client;
    static double s_stamp = 0.0;
    struct DebugLobbyListener : public LobbyListener
    {
        ~DebugLobbyListener() override {}

        virtual void onLobbyList(NetworkClient* sender, LobbyQueryResultPacket* response, double timestamp) override
        {
            (void)sender; (void)timestamp;
            fprintf(stderr, "Lobby count: %d\n", response->m_lobbyCount);
            for (int32_t i = 0; i < response->m_lobbyCount; ++i)
            {
                fprintf(stderr, "\t%s, uid=%" PRIx64 "\n", response->m_lobbynames[i], response->m_lobbySalt[i]);
            }

            if (response->m_lobbyCount == 0)
            {
                static const char s_debugLobbyName[] = "perkele";
                fprintf(stderr, "Requesting to create lobby \"%s\"\n", s_debugLobbyName);
                sender->requestCreateLobby(s_debugLobbyName, s_stamp);
            }
            else
            {
                fprintf(stderr, "Trying to join lobby %" PRIx64 "\n", response->m_lobbySalt[0]);
                // as debug join the first one
                sender->requestJoinLobby(response->m_lobbySalt[0], timestamp);
            }
        }

        virtual void onLobbyCreationResult(NetworkClient* sender, LobbyResultPacket* response, double timestamp) override
        {
            BX_UNUSED(sender, response, timestamp);
            if (response->m_created)
            {
                fprintf(stderr, "Created lobby (salt = %" PRIx64 ")\n", response->m_lobbySalt);

                fprintf(stderr, "Trying to join lobby %" PRIx64 "\n", response->m_lobbySalt);

                sender->requestJoinLobby(response->m_lobbySalt, timestamp);
            }
            else
            {
                // TODO reason
                fprintf(stderr, "Failed to create lobby\n");
            }
        }

        virtual void onLobbyJoinResult(NetworkClient* sender, LobbyJoinResultPacket* response, double timestamp) override
        {
            BX_UNUSED(sender, response, timestamp);

            if (response->m_joined)
            {
                fprintf(stderr, "Joined lobby\n");
            } 
            else
            {
                fprintf(stderr, "Failed to join lobby: %s\n", 
                    (response->m_reason == LobbyJoinResultPacket::LobbyDoesNotExist) 
                    ? "lobby does not exist" : "already joined");
            }
        }
    };
	static DebugLobbyListener s_lobbyListener;
	static Animator* anime;


	// Functions for debugging animations.
	static void playDeathAnimation(const void*)
	{
		anime->m_animator.playDeathAnimation(1, 100.0f);
	}

	static void resetAnimations(const void*)
	{
		anime->m_animator.resetAnimation();
	}

	static void climb(const void*)
	{
		anime->m_animator.playClimbAnimation(0);
	}

    static void connect(const void*)
    {
        if (s_client->isConnected()) return;
        if (s_client->isConnecting()) return;
		std::string ip = "localhost";
		uint16_t port = 8088;
		loadServerInfoFromFile(ip, port);
        s_client->connect(ip.c_str(), port, s_stamp);
        s_client->queryLobbies(s_stamp);
    }

    static void disconnect(const void*)
    {
        if (!s_client->isConnected()) return;

        s_client->disconnect(s_stamp);
    }

    static const InputBinding s_bindings[] =
    {
		{ arena::Key::KeyH, arena::Modifier::None, 0, playDeathAnimation, "debugdie" },
		{ arena::Key::KeyV, arena::Modifier::None, 0, resetAnimations, "debugAnimationReset" },
		{ arena::Key::KeyA, arena::Modifier::None, 0, inputMoveLeft, "moveleft" },
		{ arena::Key::KeyD, arena::Modifier::None, 0, inputMoveRight, "moveright" },
		{ arena::Key::KeyW, arena::Modifier::None, 0, inputMoveUp, "moveup" },
		{ arena::Key::KeyS, arena::Modifier::None, 0, inputMoveDown, "movedown" },
		{ arena::Key::Key1, arena::Modifier::None, 0, inputShoot, "shoot" },
        { arena::Key::KeyQ, arena::Modifier::None, 0, connect, "connect" },
        { arena::Key::Key9, arena::Modifier::None, 0, disconnect, "disconnect" },
		{ arena::Key::KeyR, arena::Modifier::None, 0, inputReload, "reload"},
		{ arena::Key::KeyT, arena::Modifier::None, 0, inputThrow, "apple" },
		{ arena::Key::KeyC, arena::Modifier::None, 0, climb, "climb" },
		{ arena::Key::Space, arena::Modifier::None, 0, inputJump, "jump" },

        INPUT_BINDING_END
    };


	static void inputMoveLeft(const void*)
	{
		anime->m_animator.setFlipX(false);
		sandbox->m_controller.m_input.m_leftButtonDown = true;
	}
	static void inputMoveRight(const void*)
	{
		anime->m_animator.setFlipX(true);
        sandbox->m_controller.m_input.m_rightButtonDown = true;
	}
	static void inputMoveUp(const void*)
	{
        sandbox->m_controller.m_input.m_upButtonDown = true;
	}
	static void inputMoveDown(const void*)
	{
		sandbox->m_controller.m_input.m_downButtonDown = true;
	}

	static void inputShoot(const void*)
	{
        sandbox->m_controller.m_input.m_shootButtonDown = true;
	}
	static void inputReload(const void*)
	{
		sandbox->m_controller.m_input.m_reloadButtonDown = true;
		anime->m_animator.playReloadAnimation(0);
	}
	static void inputJump(const void*)
	{
		sandbox->m_controller.m_input.m_jumpButtonDown = true;
	}
	static void inputThrow(const void*)
	{
		anime->m_animator.playThrowAnimation(0, 0);
		sandbox->m_controller.m_input.m_grenadeButtonDown = true;
	}

	SandboxScene::SandboxScene() : Scene("sandbox")
	{
		// Pointer to scene for input to use.
		sandbox = this;
		m_sendInputToServerTimer = 0;
		m_controller.aimAngle = 0;

		// Set background. This is to reduce loading time when debugging.
		// 0 = no background and no foreground, 1 = foreground, 2 = background, 3 = foreground and background.
		m_backgroundSetting = 1;
		createBackground();

		// Create gladiator for graphics debugging. This is to debug graphics without connecting to the server.
		Gladiator* glad = new Gladiator();
		*glad->m_position = glm::vec2(200, 200);
		glad->m_ownerId = 0;
		m_playerId = 0;
		createGladiator(glad);
		anime = m_clientIdToGladiatorData[m_playerId]->m_animator;
		m_scoreboard = nullptr;
		
	}

	void SandboxScene::onUpdate(const GameTime& gameTime)
	{
		s_stamp = gameTime.m_total;

		// Send packets related to finding match.
		s_client->sendMatchMakingPackets(gameTime.m_total);
		// Send packets related to connection upkeep.
		s_client->sendProtocolPackets(gameTime.m_total);

		// Send player input to server if 1/60 of a second has passed.
		if ((m_sendInputToServerTimer += gameTime.m_delta) > 0.016f && s_client->isConnected())
		{
			sendInput(m_controller);
			m_sendInputToServerTimer = 0;
		}

		// Write current packets to network.
		s_client->writePackets();
		// Get packets form network.
		s_client->readPackets();
		// Process packets stored in s_client.
		processAllPackets(gameTime);

		// Update transform component of debug bullets and delete old bullets.
		updateDebugBullets(gameTime);
		// Update all game entities.
		updateEntities(gameTime);

		updatePhysics(gameTime);

		// set m_controller aim angle of the player character.
		rotatePlayerAim();

		// rotate all gladiators aim for draw.
		for (const auto& elem : m_clientIdToGladiatorData)
		{
			elem.second->m_animator->rotateAimTo(elem.second->m_gladiator->m_aimAngle);
		}

		updateCameraPosition();
		bgfx::dbgTextClear();

		SpriteManager::instance().update(gameTime);
		AnimatorManager::instance().update(gameTime);
		
		// Set current debug draw text.
		setDrawText(gameTime);

        App::instance().spriteBatch()->submit(0);
    }
	void SandboxScene::onInitialize()
	{
        srand((uint32_t)time(NULL));
        s_client = new NetworkClient();
        s_client->m_lobbyListener = &s_lobbyListener;
		m_playerId = 0;

        inputAddBindings("player", s_bindings);
		mousePointerEntity = createMousePointerEntity();
	}
	void SandboxScene::onDestroy()
	{
		inputRemoveBindings("player");
	}
	
	void SandboxScene::sendInput(PlayerController &controller)
	{
        GameInputPacket* packet = (GameInputPacket*)createPacket(PacketTypes::GameInput);
		packet->m_aimAngle = controller.aimAngle;
        packet->m_input = controller.m_input;

		packet->m_clientSalt = s_client->m_clientSalt;
        packet->m_challengeSalt = s_client->m_challengeSalt;

		s_client->sendPacketToServer(packet, s_stamp);
		memset(&m_controller.m_input, false, sizeof(PlayerInput));
		
	}
	void SandboxScene::processAllPackets(const GameTime& gameTime)
	{
		Packet* packet = nullptr;
		ENetPeer* from;
		while ((packet = s_client->receivePacket(from)) != nullptr)
		{
			// "protocol" messages
			if (packet->getType() <= PacketTypes::Disconnect)
			{
				s_client->processClientSidePackets(packet, from, gameTime.m_total);
			}
			else if (packet->getType() <= PacketTypes::LobbyJoinResult)
			{
				s_client->processMatchmakingPackets(packet, from, gameTime.m_total);
			}
			else
				processPacket(packet);
		
			destroyPacket(packet);
		}
	}
	void SandboxScene::processPacket(Packet* packet)
	{
		switch (packet->getType())
		{
		case PacketTypes::GameSetup:
		{
			GameSetupPacket* setupPacket = (GameSetupPacket*)packet;
			// Destroy the graphics debug gladiator.
			//m_clientIdToGladiatorData[m_playerId]->destroy();
			m_clientIdToGladiatorData.clear();
			m_playerId = setupPacket->m_clientIndex;
			break;
		}
		case PacketTypes::GameUpdate:
		{
			updateGladiators((GameUpdatePacket*)packet);

			break;
		}
		case PacketTypes::GameCreateGladiators:
		{
			createGladiators((GameCreateGladiatorsPacket*)packet);
		}
		case PacketTypes::GamePlatform:
		{
			createPlatform((GamePlatformPacket*)packet);
			break;
		}
		case PacketTypes::GameSpawnBullets:
		{
			spawnBullets((GameSpawnBulletsPacket*)packet);
			break;
		}
		case PacketTypes::GameBulletHit:
		{
			spawnBulletHits((GameBulletHitPacket*)packet);
			break;
		}
		case PacketTypes::GameDamagePlayer:
		{
			processDamagePlayer((GameDamagePlayerPacket*)packet);
			break;
		}
		case PacketTypes::GameKillPlayer:
		{
			killPlayer((GameKillPlayerPacket*)packet);
			break;
		}
		case PacketTypes::GameRespawnPlayer:
		{
			respawnPlayer((GameRespawnPlayerPacket*)packet);
			break;
		}
		case PacketTypes::GameUpdateScoreBoard:
		{
			updateScoreBoard((GameUpdateScoreBoardPacket*)packet);
			break;
		}
		default:
		{
			printf("Unknown packet type received on sandbox_scene, id: %d\n", packet->getType());
		}
		}
	}

	void SandboxScene::createGladiators(GameCreateGladiatorsPacket* packet)
	{
		m_scoreboard = new Scoreboard;
		for (unsigned i = 0; i < unsigned(packet->m_playerAmount); i++)
		{
			CharacterData* characterData = &packet->m_characterArray[i];
			Gladiator* gladiator = new Gladiator();
			gladiator->m_ownerId = characterData->m_ownerId;
			gladiator->m_alive = true;
			gladiator->m_hitpoints = 100;
			gladiator->m_position =  &characterData->m_position;
			gladiator->m_aimAngle = characterData->m_aimAngle;
			gladiator->m_velocity = &characterData->m_velocity;
			Weapon* weapon = new WeaponGladius();
			gladiator->m_weapon = weapon;
			createGladiator(gladiator);
			PlayerScore playerScore;
			playerScore.m_playerID = gladiator->m_ownerId;
			playerScore.m_kills = 0;
			playerScore.m_score = 0;
			playerScore.m_tickets = 0;
			m_scoreboard->m_playerScoreVector.push_back(playerScore);
		}

		anime = m_clientIdToGladiatorData[m_playerId]->m_animator;
	}
	void SandboxScene::createPlatform(GamePlatformPacket* packet)
	{
		ArenaPlatform platform;
		platform.type = (ArenaPlatformType)packet->m_platform.m_type;
		for (unsigned i = 0; i < packet->m_platform.m_vertexAmount; i++)
		{
			platform.vertices.push_back(packet->m_platform.m_vertexArray[i]);
		}
		m_platformVector.push_back(platform);

	}
	void SandboxScene::updateGladiators(GameUpdatePacket* packet)
	{
		for (int32_t i = 0; i < packet->m_playerAmount; i++)
		{
			uint8_t playerId = packet->m_characterArray[i].m_ownerId;
			GladiatorDrawData* gladiatorData = m_clientIdToGladiatorData[playerId];
			*gladiatorData->m_gladiator->m_position = packet->m_characterArray[i].m_position;
			*gladiatorData->m_gladiator->m_velocity = packet->m_characterArray[i].m_velocity;
			// Maybe move this to entity-update.
			gladiatorData->m_transform->m_position= glm::vec2(packet->m_characterArray[i].m_position.x, packet->m_characterArray[i].m_position.y-64.0f);
			*gladiatorData->m_gladiator->m_velocity = packet->m_characterArray[i].m_velocity;
			gladiatorData->m_gladiator->m_aimAngle = packet->m_characterArray[i].m_aimAngle;

			if (packet->m_characterArray[i].m_reloading)
			{
				gladiatorData->m_animator->m_animator.playReloadAnimation(0);
			}
			if (packet->m_characterArray[i].m_throwing)
			{
				gladiatorData->m_animator->m_animator.playThrowAnimation(0,0);
			}
			if (packet->m_characterArray[i].m_climbing != 0)
			{
				gladiatorData->m_animator->m_animator.playClimbAnimation(packet->m_characterArray[i].m_climbing);
			}
			else
			{
				gladiatorData->m_animator->m_animator.endClimbAnimation();
			}

			// TODO: move this to entity-update.
			float moveSpeed = packet->m_characterArray[i].m_velocity.x;
			// Max movement speed is 300.
			if (moveSpeed < -25.0f)
			{
				gladiatorData->m_animator->m_animator.setFlipX(0);
				gladiatorData->m_animator->m_animator.startRunningAnimation(fabs(moveSpeed / 300.0f));
			}
			else if (moveSpeed > 25.0f)
			{ 
				
				gladiatorData->m_animator->m_animator.setFlipX(1);
				gladiatorData->m_animator->m_animator.startRunningAnimation(fabs(moveSpeed / 300.0f));
			}
			else
			{
				gladiatorData->m_animator->m_animator.stopRunningAnimation();
			}
		}
		
	}
	void SandboxScene::spawnBullets(GameSpawnBulletsPacket* packet)
	{
		for (unsigned i = 0; i < packet->m_bulletAmount; i++)
		{
			// Check if bullet exists.
			std::map<uint8_t, DebugBullet>::iterator it;
			it = m_debugBullets.find(packet->m_bulletSpawnArray[i].m_id);
	
			if (it != m_debugBullets.end())
			{
				// if bullet exists, set position
				*it->second.bullet->m_position = packet->m_bulletSpawnArray[i].m_position;
				//printf("Match found for id:[packet] %d \t[stored id] %d \t [key] %d \n", packet->m_bulletSpawnArray[i].m_id, it->second.bullet->m_bulletId, it->first);
				
			}
			else
				createBullet(packet->m_bulletSpawnArray[i]);
		}
	}
	void SandboxScene::spawnBulletHits(GameBulletHitPacket *packet)
	{
		for (unsigned i = 0; i < packet->m_bulletAmount; i++)
		{
			createBulletHit(packet->bulletHitArray[i]);
			destroyBullet(packet->bulletHitArray[i].m_id);
		}

	}
	void SandboxScene::processDamagePlayer(GameDamagePlayerPacket* packet)
	{
		GladiatorDrawData *gladiator = m_clientIdToGladiatorData[packet->m_targetID];
		gladiator->m_gladiator->m_hitpoints -= int32(packet->m_damageAmount);
		if (gladiator->m_gladiator->m_hitpoints <= 0)
		{
			gladiator->m_animator->m_animator.playDeathAnimation(packet->m_hitDirection > 0, packet->m_hitPosition.y);
		}

		Bullet bullet;
		bullet.m_position->x = packet->m_hitPosition.x;
		bullet.m_position->y = packet->m_hitPosition.y;
		destroyBullet(packet->m_bulletId);
		createBloodBulletHitEntity(bullet);

		// Todo: Set animation blood on hit position. Draw blood on gladiator.

	}
	void SandboxScene::killPlayer(GameKillPlayerPacket* packet)
	{
		m_clientIdToGladiatorData[packet->m_playerID]->m_gladiator->m_hitpoints = 0;
		m_clientIdToGladiatorData[packet->m_playerID]->m_gladiator->m_alive = false;
	}
	void SandboxScene::respawnPlayer(GameRespawnPlayerPacket* packet)
	{
		m_clientIdToGladiatorData[packet->m_playerID]->m_gladiator->m_hitpoints = 100;
		m_clientIdToGladiatorData[packet->m_playerID]->m_gladiator->m_alive = true;
		m_clientIdToGladiatorData[packet->m_playerID]->m_animator->m_animator.resetAnimation();
	}
	void SandboxScene::updateScoreBoard(GameUpdateScoreBoardPacket* packet)
	{
		m_scoreboard->m_flagHolder = packet->m_scoreBoardData.m_flagHolder;
		for (unsigned i = 0; i < packet->m_playerAmount; i++)
		{
			m_scoreboard->m_playerScoreVector[i].m_score = (packet->m_scoreBoardData.m_playerScoreArray[i].m_score);
			m_scoreboard->m_playerScoreVector[i].m_tickets = (packet->m_scoreBoardData.m_playerScoreArray[i].m_tickets);
			m_scoreboard->m_playerScoreVector[i].m_kills = (packet->m_scoreBoardData.m_playerScoreArray[i].m_kills);
		}
	}

	void SandboxScene::updateEntities(const GameTime& gameTime)
	{
		// TODO: Do own systems for these.
		Transform* playerTransform = (Transform* const)m_clientIdToGladiatorData[m_playerId]->m_entity->first(TYPEOF(Transform));
		for (EntityIterator iterator = entititesBegin(); iterator != entititesEnd(); iterator++)
		{
			Entity* entity = *iterator;
			if (entity->contains(TYPEOF(Timer)))
			{
				// Update timer and destroy entities that are out of their lifetime.
				Timer* timer = (Timer*)entity->first(TYPEOF(Timer));
				if (timer->timePassed(gameTime.m_delta))
				{
					entity->destroy();
					continue;
				}

				// Fade entities that need to be faded away
				if (entity->contains(TYPEOF(SpriteRenderer)) && entity->contains(TYPEOF(Id)))
				{
					Id* entityId = (Id*)entity->first(TYPEOF(Id));
					// If entity is type of smoke, fade it.
					if (entityId->m_id == Smoke)
					{
						SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
						render->setColor(color::toABGR(255, 255, 255, (uint8_t)timer->timePassedReverse255() / 8));
					}
				}

				// Move entities that need to be moved
				if (entity->contains(TYPEOF(Movement)) && entity->contains(TYPEOF(Transform)) && entity->contains(TYPEOF(SpriteRenderer)))
				{
					Movement* movement = (Movement*)entity->first(TYPEOF(Movement));
					Transform* transform = (Transform*)entity->first(TYPEOF(Transform));
					SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));


					if (timer->m_between > 0.016f)
					{
						transform->m_position += movement->m_velocity;
						movement->m_velocity = glm::vec2(movement->m_velocity.x *0.9, movement->m_velocity.y*0.9);

						render->setRotation(render->getRotation() + movement->m_rotationSpeed);
						timer->resetBetween();
					}
				}

			}
			if (entity->contains(TYPEOF(Id)))
			{


				Id* id = (Id*)entity->first(TYPEOF(Id));
				if (id->m_id == MapBack)
				{
					Movement* movement = (Movement*)entity->first(TYPEOF(Movement));
					Transform* transform = (Transform*)entity->first(TYPEOF(Transform));
					// Parallex scrolling.
					// use movement component as offset component.
					glm::vec2 mapOffset = glm::vec2(playerTransform->m_position.x *1.2f, playerTransform->m_position.y *1.2f);
					// HAX: velocity is original position, too lazy to create originalposition-component or something like that
					transform->m_position = glm::vec2(movement->m_velocity.x + mapOffset.x, movement->m_velocity.y + mapOffset.y);
				}
				else if (id->m_id == HitBlood)
				{
					//Movement* movement = (Movement*)entity->first(TYPEOF(Movement));
					//					Transform* transform = (Transform*)entity->first(TYPEOF(Transform));
					SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
					Timer* timer = (Timer*)entity->first(TYPEOF(Timer));

					// TODO: Remake this.
					float time = timer->timePassed();

					int multipler = 0;

					if (time < 0.10f)
					{
						multipler = 0;
					}
					else if (time < 0.25)
					{
						multipler = 1;
					}
					else if (time < 0.45)
					{
						multipler = 2;
					}
					else if (time < 0.55)
					{
						multipler = 3;
					}
					else if (time < 0.75)
					{
						multipler = 4;
					}

					else if (time < 0.85)
					{
						multipler = 5;
					}

					else if (time < 0.90)
					{
						multipler = 6;
					}

					Rectf& rectangle = render->getSource();
					rectangle.x = 128.0f * multipler;
					rectangle.y = 0.0f;
					rectangle.w = 128.0f * (multipler + 1);
					rectangle.h = 32.0f;
					
					rectangle = render->getSource();

				};

			}
		}
	}
	void SandboxScene::updateDebugBullets(const GameTime& gameTime)
	{
		for (std::map<uint8_t, DebugBullet>::iterator it = m_debugBullets.begin(); it != m_debugBullets.end(); )
		{
			if ((it->second.lifeTime += gameTime.m_delta) < 10.0f)
			{
				Transform* bulletTransform = (Transform* const)it->second.entity->first(TYPEOF(Transform));
				bulletTransform->m_position = *it->second.bullet->m_position;
				++it;
			}
			else
			{
				// If bullets for some reason life longer than ten secounds, destroy them.
				it->second.entity->destroy();
				it->second.destroy();
				it = m_debugBullets.erase(it);
			}
		}
	}
	
	void SandboxScene::updatePhysics(const GameTime& gameTime)
	{
		m_physics.update(gameTime.m_delta);
	}

	Entity* SandboxScene::createMousePointerEntity()
	{
		EntityBuilder builder;
		builder.begin();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = glm::vec2(0, 0);

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/crosshair.png"));
		glm::vec2& origin = renderer->getOrigin();
		origin.x = 16.0f; origin.y = 16.0f;
		renderer->anchor();

		builder.addIdentifier(EntityIdentification::MousePointer);
		Entity* mousePointer = builder.getResults();
		registerEntity(mousePointer);
		return mousePointer;
	}

	void SandboxScene::destroyBullet(uint8_t bulletId)
	{
		for (std::map<uint8_t, DebugBullet>::iterator it = m_debugBullets.begin(); it != m_debugBullets.end(); )
		{
			if (it->second.bullet->m_bulletId == bulletId)
			{
				it->second.lifeTime = 20;
				//it->second.entity->destroy();
				//it->second.destroy();
				//it = m_debugBullets.erase(it);
				return;
			}
			it++;
		}
	}

	void SandboxScene::createGladiator(Gladiator* gladiator)
	{
		Entity* entity_gladiator;
		EntityBuilder builder;
		builder.begin();

		ResourceManager* resources = App::instance().resources();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = *gladiator->m_position;

		Animator* animator = builder.addCharacterAnimator();
		CharacterAnimator& anim = animator->m_animator;
	
		anim.setStaticContent(
			resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Crest4.png"),
			resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Helmet.png"),
			resources->get<TextureResource>(ResourceType::Texture, "Characters/body/1_Torso.png"),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/LegAnimations/RunStandJump.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/DyingAndClimbingAnimations/Dying.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/ReloadingAndThrowingAnimations/ThrowingGrenade.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/ReloadingAndThrowingAnimations/Gladius.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/ReloadingAndThrowingAnimations/Axe.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/DyingAndClimbingAnimations/Climbing.scml")->getNewEntityInstance(0)

		);
		anim.setWeaponAnimation(WeaponAnimationType::Gladius);
		
		entity_gladiator = builder.getResults();

		registerEntity(entity_gladiator);

		GladiatorDrawData* data = new GladiatorDrawData;
		data->m_entity = entity_gladiator;
		data->m_animator = animator;
		data->m_transform = transform;
		data->m_gladiator = new Gladiator();
		
		m_clientIdToGladiatorData.insert(std::pair<uint8_t, GladiatorDrawData*>(gladiator->m_ownerId, data));

	}
	void SandboxScene::createBullet(BulletData &data)
	{
		// TODO: add some identifier (Color?) on bullets, to see clientside/serverside difference.

		// Create bullet entity that is updated by server. (DEBUG)
		Bullet* bullet = new Bullet;
		*bullet->m_position = data.m_position;
		bullet->m_bulletId = data.m_id;
		bullet->m_type = (BulletType)data.m_type;
		bullet->m_creationDelay = data.m_creationDelay;
		bullet->m_rotation = data.m_rotation;

		Entity* serverEntity = nullptr;
		switch (bullet->m_type)
		{
		case BulletType::GladiusBullet:
			serverEntity = createBulletEntity(bullet);
			break;
		case BulletType::ShotgunBullet:
			serverEntity = createBulletEntity(bullet);
			break;
		case BulletType::Grenade:
			serverEntity = createGrenadeEntity(bullet);
			break;
		default:
			serverEntity = createBulletEntity(bullet);
			break;
		}
		assert(serverEntity != nullptr);
		DebugBullet debugBullet;
		debugBullet.bullet = bullet;
		debugBullet.entity = serverEntity;
		m_debugBullets.insert(std::pair<uint8_t, DebugBullet>(debugBullet.bullet->m_bulletId, debugBullet));
		


		// Create bullet entity that is updated by clientside physics
		Entity* clientEntity = nullptr;
		switch (bullet->m_type)
		{
		case BulletType::GladiusBullet:
			clientEntity = createBulletEntity(bullet);
			break;
		case BulletType::ShotgunBullet:
			clientEntity = createBulletEntity(bullet);
			break;
		case BulletType::Grenade:
			clientEntity = createGrenadeEntity(bullet);
			break;
		default:
			clientEntity = createBulletEntity(bullet);
			break;
		}
		assert(clientEntity != nullptr);

		Transform* transform = (Transform*)clientEntity->first(TYPEOF(Transform));
		Projectile* projectile = (Projectile*)clientEntity->first(TYPEOF(Projectile));
		projectile->m_bulletId = bullet->m_bulletId;
		projectile->m_bulletType = bullet->m_type;
		glm::vec2 force = radToVec(bullet->m_rotation);
		// TODO: Get real player id for client-side collisions. If bullets get removed after being shot by
		// any other player than 0, it's because of this.
		m_physics.addClientSideBullet(&transform->m_position, glm::vec2(force.x*20.0f, force.y * 20.0f), 0, bullet->m_bulletId);
		
		// TODO: Update clientside bullet on updateEntities();
	}
	Entity* SandboxScene::createBulletEntity(Bullet* bullet)
	{
		EntityBuilder builder;
		builder.begin();

		// Debugbullet does not need projectile, but clientside physics needs it 
		// for the projectile to be deleted by server.
		builder.addProjectile();
		Transform* transform = builder.addTransformComponent();
		transform->m_position = *bullet->m_position;
		
		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();
		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "bullet_placeholder1.png"));
		//if (bullet->m_rotation < 3.142)
		renderer->setRotation(bullet->m_rotation);
		//else
		//renderer->setRotation(bullet->m_rotation+ 3.142);
		
		renderer->anchor();
		// TODO: add physics for clientside bullets.
		//Movement* movement = builder.addMovement();
		//movement->m_velocity = bullet->m_impulse;
		Entity* entity = builder.getResults();
		registerEntity(entity);
		
		// effects		
		createMuzzleFlashEntity(*bullet);
		createSmokeEntity(*bullet);
		return entity;
	}
	Entity* SandboxScene::createGrenadeEntity(Bullet* bullet)
	{

		EntityBuilder builder;
		builder.begin();
		builder.addProjectile();
		Transform* transform = builder.addTransformComponent();
		transform->m_position = *bullet->m_position;

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/grenade.png"));
		renderer->setRotation(bullet->m_rotation);
		
		renderer->anchor();
		Movement* movement = builder.addMovement();
		movement->m_velocity = glm::vec2(0, 0);
		movement->m_rotationSpeed = 0.02f;

		Timer* timer = builder.addTimer();
		// Let debugBullets/serve decide when grenade is destroyed.
		timer->m_lifeTime = 70;

		Entity* entity = builder.getResults();
		registerEntity(entity);

		return entity;
	}

	void SandboxScene::createMuzzleFlashEntity(const Bullet& bullet)
	{
		EntityBuilder builder;
		builder.begin();
		Transform* transform = builder.addTransformComponent();
		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		Timer* timer = builder.addTimer();
		timer->m_lifeTime = 0.05f;
		builder.addIdentifier(EntityIdentification::MuzzleFlash);
		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/muzzleFlash_ss.png"));
		Rectf& source = renderer->getSource();

		if (m_nextSprite < 3)
			m_nextSprite++;
		else m_nextSprite = 0;

		source.x = 0.0f + (float)m_nextSprite * 32.0f;
		source.y = 0.0f;
		source.w = 32.0f;
		source.h = 32.0f;

		// Bullet entity is updated once before sending, so it's is no in creation position.
		// This should be fixed later, but for now we need to backtrack the bullet position a bit.
		glm::vec2 angleAsVector = radToVec(bullet.m_rotation);
		glm::vec2 backTrackedBulletPosition = glm::vec2(bullet.m_position->x - angleAsVector.x * 25, bullet.m_position->y - angleAsVector.y * 25);

		transform->m_position = glm::vec2(backTrackedBulletPosition.x - 16, backTrackedBulletPosition.y - 16);
		glm::vec2& origin = renderer->getOrigin();
		origin.x = origin.x + 16; origin.y = origin.y + 16;
		renderer->setRotation((float32)bullet.m_rotation + 3.142f);
		renderer->setLayer(2);
		renderer->anchor();
		registerEntity(builder.getResults());
	}
	void SandboxScene::createSmokeEntity(const Bullet& bullet)
	{
		EntityBuilder builder;
		builder.begin();
		ResourceManager* resources = App::instance().resources();
		(void)resources;

		Timer* timer = builder.addTimer();

		// Load gun smoke, randomize rotation and position on spawn.

		for (int i = 0; i < rand() % 5 + 3; i++)
		{
			int spriteX = rand() % 4;
			
			float rotation = 0;
			int xOffset = 0, yOffset = 0;

			builder.begin();

			builder.addIdentifier(EntityIdentification::Smoke);
			// Timer
			timer = builder.addTimer();
			timer->m_lifeTime = 0.5f;

			// Drawing stuff
			Transform* transform = builder.addTransformComponent();
			SpriteRenderer* renderer = builder.addSpriteRenderer();
			renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/gunSmoke1_ss.png"));
			//uint32_t color = color::toABGR(255, 255, 255, 50);
			//renderer->setColor(color);
			glm::vec2& origin = renderer->getOrigin();
			origin.x = origin.x + 16; origin.y = origin.y + 16;
			renderer->setRotation(float32(rand() % 7));
			Rectf& source = renderer->getSource();
			source.x = 32 * (float)spriteX; source.y = 0; source.w = 32; source.h = 32;


			if (rand() % 2 == 1) {
				xOffset = rand() % 10;
				rotation = (rand() % 3) / 100.0f;
			}
			else {
				xOffset = -(rand() % 10);
				rotation = (float)-(rand() % 3) / 100.0f;
			}
			if (rand() % 2 == 1) {
				yOffset = rand() % 10;
			}
			else {
				yOffset = -(rand() % 10);
			}

			//transform->m_position = glm::vec2(bullet->m_position->x+xOffset-16, bullet->m_position->y+yOffset-16);
			transform->m_position = glm::vec2(bullet.m_position->x - 16, bullet.m_position->y - 16);

			// Movement
			Movement* movement = builder.addMovement();
			//movement->m_velocity = glm::vec2(float(xOffset)/100.0f, float(yOffset) / 100.0f);
			movement->m_velocity = glm::vec2(cos(bullet.m_rotation) * 2 + float(xOffset) / 5.0f, sin(bullet.m_rotation) * 2 + float(yOffset) / 5.0f);
			movement->m_rotationSpeed = rotation;

			renderer->anchor();
			registerEntity(builder.getResults());
		} 

	}

	void SandboxScene::createBulletHit(BulletHitData& data)
	{
		Bullet bullet;
		*bullet.m_position = data.m_position;
		bullet.m_type = (BulletType)data.m_type;
		bullet.m_rotation = data.m_rotation;

		switch (data.m_type)
		{
			case 1:
			{
				createBloodBulletHitEntity(bullet);
				data.m_id;
				break;
			}
			case 2:
			{
				// Temporary, change when there is a platform bullet hit animation.
				createSmokeEntity(bullet);
				//createPlatformBulletHitEntity(bullet);
				break;
			}
			default:
			{
				break;
			}
		}
		
		
	}
	void SandboxScene::createBloodBulletHitEntity(Bullet& bullet)
	{
		EntityBuilder builder;
		builder.begin();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = *bullet.m_position + bullet.m_rotation;

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/bloodPenetrationAnimation1_ss.png"));
		Rectf rect = renderer->getSource();
		renderer->setSize(128, 32);
		rect.x = 0; rect.y = 0;
		rect.w = 128; rect.h = 32;
		renderer->anchor();

		Timer* timer =builder.addTimer();
		timer->m_lifeTime = 0.5f;

		Movement* move = builder.addMovement();
		move->m_velocity = glm::vec2(bullet.m_impulse.x, bullet.m_impulse.y);
		
		builder.addIdentifier(EntityIdentification::HitBlood);
		registerEntity(builder.getResults());
	}
	void  SandboxScene::createPlatformBulletHitEntity(Bullet& bullet)
	{
		EntityBuilder builder;
		builder.begin();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = *bullet.m_position + bullet.m_rotation;

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/bloodPenetrationAnimation1_ss.png"));
		Rectf rect = renderer->getSource();
		renderer->setSize(128, 32);
		rect.x = 0; rect.y = 0;
		rect.w = 128; rect.h = 32;
		renderer->anchor();

		Timer* timer = builder.addTimer();
		timer->m_lifeTime = 0.5f;

		Movement* move = builder.addMovement();
		move->m_velocity = glm::vec2(bullet.m_impulse.x, bullet.m_impulse.y);

		builder.addIdentifier(EntityIdentification::HitBlood);
		registerEntity(builder.getResults());
	}

	void SandboxScene::checkBounds(glm::vec2& cameraPosition)
	{
		if (cameraPosition.x < m_screenSize.x / 2)
		{
			cameraPosition.x = m_screenSize.x / 2;
		}
		else if (cameraPosition.x > m_screenSize.x * 3.5f)
		{
			cameraPosition.x = m_screenSize.x * 3.5f;
		}
		if (cameraPosition.y < m_screenSize.y / 2)
		{
			cameraPosition.y = m_screenSize.y / 2;
		}
		else if (cameraPosition.y > m_screenSize.y * 1.5f )
		{
			cameraPosition.y = m_screenSize.y* 1.5f;
		}
	}

	void SandboxScene::updateCameraPosition()
	{
		Camera& camera = App::instance().camera();
		if(m_clientIdToGladiatorData.at(m_playerId) != nullptr)
		{ 
			Transform* playerTransform = (Transform* const)m_clientIdToGladiatorData[m_playerId]->m_entity->first(TYPEOF(Transform));
			Transform* mouseTransform = (Transform* const)mousePointerEntity->first(TYPEOF(Transform));
			const MouseState& mouse = Mouse::getState();
			// TODO: get real resolution
			
			
			glm::vec2 cameraPositionOnMouse = glm::vec2(-m_screenSize.x/2 + mouse.m_mx, -m_screenSize.y/2 + mouse.m_my);
			glm::vec2 movement = cameraPositionOnMouse + oldMousePos;
			glm::vec2 cameraPosition = glm::vec2(oldMousePos.x + movement.x + playerTransform->m_position.x, oldMousePos.y + movement.y + playerTransform->m_position.y);
			oldMousePos = cameraPositionOnMouse;
			// Adjust the aim position where bullets drop.
			mouseTransform->m_position = cameraPosition + glm::vec2(-16, 36.0f);
			//checkBounds(cameraPosition);
			camera.m_position = cameraPosition;
			camera.calculate();
			// set views
			float ortho[16];
			bx::mtxOrtho(ortho, 0.0f, float(camera.m_bounds.x), float(camera.m_bounds.y), 0.0f, 0.0f, 1000.0f);
			bgfx::setViewTransform(0, glm::value_ptr(camera.m_matrix), ortho);
			bgfx::setViewRect(0, 0, 0, uint16_t(camera.m_bounds.x), uint16_t(camera.m_bounds.y));

			bgfx::setViewTransform(1, glm::value_ptr(camera.m_matrix), ortho);
			bgfx::setViewRect(1, 0, 0, uint16_t(camera.m_bounds.x), uint16_t(camera.m_bounds.y));
		}
	}
	void SandboxScene::rotatePlayerAim()
	{
		const MouseState& mouse = Mouse::getState();

		glm::vec2 mouseLoc(mouse.m_mx, mouse.m_my);
		Camera& camera = App::instance().camera();
		transform(mouseLoc, glm::inverse(camera.m_matrix), &mouseLoc);
		Transform* playerTransform = (Transform* const)m_clientIdToGladiatorData[m_playerId]->m_entity->first(TYPEOF(Transform));
		glm::vec2 weaponRotationPoint = glm::vec2(playerTransform->m_position.x + 8, playerTransform->m_position.y + 28);
		glm::vec2 dir(mouseLoc - weaponRotationPoint);
		float a = glm::atan(dir.y, dir.x);
		m_controller.aimAngle = a;
		SpriteRenderer* mouseTransform = (SpriteRenderer* const)mousePointerEntity->first(TYPEOF(SpriteRenderer));
		mouseTransform->setRotation(a + 3.14f/2);
		// Update own gladiator aim
		m_clientIdToGladiatorData[m_playerId]->m_gladiator->m_aimAngle = m_controller.aimAngle;
	}
	void SandboxScene::setDrawText(const GameTime& gameTime)
	{
		unsigned row = 0;
		gameTime;
		//const MouseState& mouse = Mouse::getState();
		//glm::vec2 mouseLoc(mouse.m_mx, mouse.m_my);
		//bgfx::dbgTextPrintf(0, row, 0x9f, "Delta time %.10f", gameTime.m_delta);
		//row++;
		//bgfx::dbgTextPrintf(0, row, 0x8f, "Left btn = %s, Middle btn = %s, Right btn = %s",
		//row++;
		//	mouse.m_buttons[MouseButton::Left] ? "down" : "up",
		//	mouse.m_buttons[MouseButton::Middle] ? "down" : "up",
		//	mouse.m_buttons[MouseButton::Right] ? "down" : "up");
		//bgfx::dbgTextPrintf(0, row, 0x6f, "Mouse (screen) x = %d, y = %d, wheel = %d", mouse.m_mx, mouse.m_my, mouse.m_mz);
		//row++;
		//bgfx::dbgTextPrintf(0, row, 0x9f, "Mouse pos (world) x= %.2f, y=%.2f", mouseLoc.x, mouseLoc.y);
		//row++;
		//bgfx::dbgTextPrintf(0, row, 0x9f, "Angle (%.3f rad) (%.2f deg)", m_controller.aimAngle, glm::degrees(m_controller.aimAngle));
		
		if (m_scoreboard == nullptr)
			return;
		for (const auto& elem : m_scoreboard->m_playerScoreVector)
		{
			bgfx::dbgTextPrintf(0, row, 0x8f, "Player: %d: \t Score: %d \t Kills: %d \t Tickets %d",
				elem.m_playerID, elem.m_score, elem.m_kills, elem.m_tickets);
			row++;
		}
	}
	void SandboxScene::createBackground()
	{
		ResourceManager* resources = App::instance().resources();
		(void)resources;
		EntityBuilder builder;

		// Create back
		if (m_backgroundSetting > 1)
		{ 
		
			for (int y = 0; y <= 3; y++)
			{
				for (int x = 0; x <= 5; x++)
				{
					// TODO: add offset component.
					builder.begin();
					Movement* move= builder.addMovement();
					move->m_velocity = glm::vec2(0, 0);
					move->m_rotationSpeed = 0;
					builder.addIdentifier(MapBack);
					std::string name = "map/B";
					name += std::to_string(x);
					name += std::to_string(y);
					name += ".png";

					SpriteRenderer* renderer = builder.addSpriteRenderer();
					TextureResource* textureResource = resources->get<TextureResource>(ResourceType::Texture, name);
					Transform* transform = builder.addTransformComponent();

					transform->m_position = glm::vec2((x-1) * 1920, (y-1) * 1080);
					renderer->setTexture(textureResource);
					move->m_velocity = transform->m_position;
					renderer->anchor();
					registerEntity(builder.getResults());
				}
			}
		}
		
		// Create front
		if (m_backgroundSetting == 1 || m_backgroundSetting == 3)
		{

			for (unsigned y = 0; y <= 1; y++)
			{
				for (unsigned x = 0; x <= 3; x++)
				{
					builder.begin();
					builder.addIdentifier(MapFront);
					SpriteRenderer* renderer = builder.addSpriteRenderer();

					std::string name = "map/";
					name += std::to_string(x);
					name += std::to_string(y);
					name += ".png";

					TextureResource* textureResource = resources->get<TextureResource>(ResourceType::Texture, name);

					Transform* transform = builder.addTransformComponent();

					transform->m_position = glm::vec2(x * 1920, y * 1080);

					renderer->setTexture(textureResource);

					renderer->anchor();
					registerEntity(builder.getResults());
				}
			}
		}
				
	}
}