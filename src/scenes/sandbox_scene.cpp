#include "../net/network_client.h"
#include "sandbox_scene.h"
#include "..\res\resource_manager.h"
#include "..\app.h"

#include "..\ecs\entity.h"
#include "..\ecs\sprite_renderer.h"
#include "..\ecs\managers\sprite_manager.h"

#include "..\game_time.h"

#include "../graphics/character_animator.h"
#include "../res/spriter_resource.h"
#include "../ecs/animator.h"
#include "..\ecs\movement.h"

#ifdef _DEBUG
#	include <iostream>
#   include "..\rtti\rtti_define.h"
#   include "..\ecs\transform.h"
#   include "..\ecs\sprite_renderer.h"
#	include "../res/texture_resource.h"
#endif

#include "..\ecs\timer.h"
#include "..\utils\color.h"
#include "..\ecs\entity_builder.h"
#include "../ecs/managers/animator_manager.h"
#include <bx/fpumath.h>
#include <glm/gtc/type_ptr.hpp>
#include "../graphics/spritebatch.h"
#include "../input/event.h"
#include "../utils/math.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <common/packet.h>
#include <common/arena/gladiator.h>
#include <time.h>
#include <stdlib.h>


namespace arena
{
	SandboxScene* sandbox;
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


    static NetworkClient* s_client;
    static DebugLobbyListener s_lobbyListener;

	static Animator* anime;
    static void left(const void*)
    {
		anime->m_animator.setFlipX(false);
    }

    static void right(const void*)
    {
		anime->m_animator.setFlipX(true);
    }
    
	static void diefool(const void*)
	{
		anime->m_animator.playDeathAnimation(1, 100.0f);
	}

	static void respawn(const void*)
	{
		anime->m_animator.resetAnimation();
	}

	static void reload(const void*)
	{
		anime->m_animator.playReloadAnimation(0);
	}

	static void throwAGrenade(const void*)
	{
		anime->m_animator.playThrowAnimation(0, 0);
	}

	static void climb(const void*)
	{
		anime->m_animator.playClimbAnimation(0);
	}

    static void connect(const void*)
    {
        if (s_client->isConnected()) return;
        if (s_client->isConnecting()) return;

        s_client->connect("localhost", uint16_t(8088), s_stamp);
        s_client->queryLobbies(s_stamp);
    }

    static void disconnect(const void*)
    {
        if (!s_client->isConnected()) return;

        s_client->disconnect(s_stamp);
    }

    static const InputBinding s_bindings[] =
    {
        { arena::Key::KeyA, arena::Modifier::None, 0, left, "left" },
        { arena::Key::KeyD, arena::Modifier::None, 0, right, "right" },
		{ arena::Key::KeyS, arena::Modifier::None, 0, diefool, "die" },
		{ arena::Key::KeyV, arena::Modifier::None, 0, respawn, "respawn" },
		{ arena::Key::KeyA, arena::Modifier::None, 0, inputMoveLeft, "moveleft" },
		{ arena::Key::KeyD, arena::Modifier::None, 0, inputMoveRight, "moveright" },
		{ arena::Key::KeyW, arena::Modifier::None, 0, inputMoveUp, "moveup" },
		{ arena::Key::Key1, arena::Modifier::None, 0, inputShoot, "shoot" },
        { arena::Key::KeyQ, arena::Modifier::None, 0, connect, "connect" },
        { arena::Key::KeyE, arena::Modifier::None, 0, disconnect, "disconnect" },
		{ arena::Key::KeyR, arena::Modifier::None, 0, reload, "reload"},
		{ arena::Key::KeyT, arena::Modifier::None, 0, throwAGrenade, "apple" },
		{ arena::Key::KeyC, arena::Modifier::None, 0, climb, "climb" },
        INPUT_BINDING_END
    };

	static void inputMoveLeft(const void*)
	{
		sandbox->m_controller.m_input.m_leftButtonDown = true;
	}
	static void inputMoveRight(const void*)
	{
        sandbox->m_controller.m_input.m_rightButtonDown = true;
	}
	static void inputMoveUp(const void*)
	{
        sandbox->m_controller.m_input.m_upButtonDown = true;
	}
	static void inputShoot(const void*)
	{
        sandbox->m_controller.m_input.m_shootButtonDown = true;
	}

	void SandboxScene::setAimAngle(float angle)
	{
		m_controller.aimAngle = angle;
	}


	SandboxScene::SandboxScene() : Scene("sandbox")
	{
		sandbox = this;
		connected = false;
		sendInputToServerTimer = 0;
		m_controller.aimAngle = 0;

		createBackground();
		Gladiator* glad = new Gladiator();
		*glad->m_position = glm::vec2(200, 200);
		glad->m_ownerId = 0;
		m_playerId = 0;
		createGladiator(glad);
		
	}

    void SandboxScene::onUpdate(const GameTime& gameTime)
    {
        s_stamp = gameTime.m_total;

        s_client->sendMatchMakingPackets(gameTime.m_total);
        s_client->sendProtocolPackets(gameTime.m_total);

        if ((sendInputToServerTimer += gameTime.m_delta) > 0.016f && connected)
        {
            sendInput(m_controller);

            memset(&m_controller.m_input, false, sizeof(PlayerInput));

            sendInputToServerTimer = 0;
        }

        s_client->writePackets();
        s_client->readPackets();



        Packet* packet = nullptr;
        ENetPeer* from;
        while ((packet = s_client->receivePacket(from)) != nullptr)
        {
            // "protocol" messages
            if (packet->getType() <= PacketTypes::Disconnect)
            {
                s_client->processClientSidePackets(packet, from, gameTime.m_total);
            }
            else
            {
                s_client->processMatchmakingPackets(packet, from, gameTime.m_total);

                switch (packet->getType())
                {
                case PacketTypes::GameSetup:
                {
                    connected = true;
                    GameSetupPacket* setupPacket = (GameSetupPacket*)packet;
                    m_playerId = setupPacket->m_clientIndex;

                    for (int32_t i = 0; i < setupPacket->m_playerAmount; i++)
                    {
                        m_scoreboard.m_playerScoreVector.push_back(PlayerScore());
                    }
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
                    GameUpdateScoreBoard((GameUpdateScoreBoardPacket*)packet);
                    break;
                }
                default:
                {
                    printf("Unknown packet type received on sandbox_scene, id: %d\n", packet->getType());
                }
                }
            }

            destroyPacket(packet);
        }

        Transform* playerTransform = (Transform* const)m_clientIdToGladiatorData[m_playerId]->m_entity->first(TYPEOF(Transform));

        for (std::map<uint8_t, DebugBullet>::iterator it = m_debugBullets.begin(); it != m_debugBullets.end(); )
        {
            if ((it->second.lifeTime += gameTime.m_delta) < 1.0f)
            {
                Transform* bulletTransform = (Transform* const)it->second.entity->first(TYPEOF(Transform));
                bulletTransform->m_position = *it->second.bullet->m_position;
				++it;
            }
            else
            {
				it->second.entity->destroy();
				it->second.destroy();
				it = m_debugBullets.erase(it);
            }
        }

		for (EntityIterator iterator = entititesBegin(); iterator != entititesEnd(); iterator++)
		{
			Entity* entity = *iterator;
			if (entity->contains(TYPEOF(Timer)))
			{
				Timer* timer = (Timer*)entity->first(TYPEOF(Timer));
				if (timer->timePassed(gameTime.m_delta))
				{
					//TODO: delete entity . Deletion is broken.
					if (entity->contains(TYPEOF(SpriteRenderer)))
					{
						SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
						render->hide();
						continue;
					}
				}
				if (entity->contains(TYPEOF(SpriteRenderer)))
				{
					SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
					render->setColor(color::toABGR(255, 255, 255, timer->timePassedReverse255()));
				}

				if (entity->contains(TYPEOF(Movement)) && entity->contains(TYPEOF(Transform)) && entity->contains(TYPEOF(SpriteRenderer)))
				{
					Movement* movement = (Movement*)entity->first(TYPEOF(Movement));
					Transform* transform = (Transform*)entity->first(TYPEOF(Transform));
					SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
					if (timer->m_between > 0.016f)
					{
						transform->m_position += movement->m_velocity;
						render->setRotation(render->getRotation() + movement->m_rotationSpeed);
						timer->resetBetween();
					}
				}
			}

		}

        Camera& camera = App::instance().camera();
        camera.m_position = playerTransform->m_position;
        camera.calculate();
        // set views
        float ortho[16];
        bx::mtxOrtho(ortho, 0.0f, float(camera.m_bounds.x), float(camera.m_bounds.y), 0.0f, 0.0f, 1000.0f);
        bgfx::setViewTransform(0, glm::value_ptr(camera.m_matrix), ortho);
        bgfx::setViewRect(0, 0, 0, uint16_t(camera.m_bounds.x), uint16_t(camera.m_bounds.y));

        bgfx::dbgTextClear();

        SpriteManager::instance().update(gameTime);
        AnimatorManager::instance().update(gameTime);

        const MouseState& mouse = Mouse::getState();

        glm::vec2 mouseLoc(mouse.m_mx, mouse.m_my);
        transform(mouseLoc, glm::inverse(camera.m_matrix), &mouseLoc);

        glm::vec2 dir(mouseLoc - playerTransform->m_position);
        float a = glm::atan(dir.y, dir.x);
        m_controller.aimAngle = a;

		

        for (const auto& elem : m_clientIdToGladiatorData)
        {
            elem.second->m_animator->rotateAimTo(elem.second->m_gladiator->m_aimAngle);
        }
        m_clientIdToGladiatorData[m_playerId]->m_animator->rotateAimTo(a);

        App::instance().spriteBatch()->submit(0);

        bgfx::dbgTextPrintf(0, 1, 0x9f, "Delta time %.10f", gameTime.m_delta);
        bgfx::dbgTextPrintf(0, 2, 0x8f, "Left btn = %s, Middle btn = %s, Right btn = %s",
            mouse.m_buttons[MouseButton::Left] ? "down" : "up",
            mouse.m_buttons[MouseButton::Middle] ? "down" : "up",
            mouse.m_buttons[MouseButton::Right] ? "down" : "up");
        bgfx::dbgTextPrintf(0, 3, 0x6f, "Mouse (screen) x = %d, y = %d, wheel = %d", mouse.m_mx, mouse.m_my, mouse.m_mz);
        bgfx::dbgTextPrintf(0, 4, 0x9f, "Mouse pos (world) x= %.2f, y=%.2f", mouseLoc.x, mouseLoc.y);
        bgfx::dbgTextPrintf(0, 5, 0x9f, "Angle (%.3f rad) (%.2f deg)", a, glm::degrees(a));

    }

	void SandboxScene::onInitialize()
	{
        srand((uint32_t)time(NULL));
        s_client = new NetworkClient();
        s_client->m_lobbyListener = &s_lobbyListener;
		m_playerId = 0;

		
        inputAddBindings("player", s_bindings);
	}
	void SandboxScene::onDestroy()
	{
		inputRemoveBindings("player");
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
		
		
		//anim.setPosition(glm::vec2(0, 0));

		entity_gladiator = builder.getResults();

		registerEntity(entity_gladiator);

		// TODO: should use entities.
		GladiatorDrawData* data = new GladiatorDrawData;
		data->m_entity = entity_gladiator;
		data->m_animator = animator;
		data->m_transform = transform;
		data->m_gladiator = new Gladiator();
		
		m_clientIdToGladiatorData.insert(std::pair<uint8_t, GladiatorDrawData*>(gladiator->m_ownerId, data));

		// TODO: do animation stuff better
		anime = m_clientIdToGladiatorData[gladiator->m_ownerId]->m_animator;
	}
	void SandboxScene::createBackground()
	{
		
        for (unsigned y = 0; y <= 1; y++)
        {
            for (unsigned x = 0; x <= 3; x++)
            {
                EntityBuilder builder;
                builder.begin();
                ResourceManager* resources = App::instance().resources();
                (void)resources;

                SpriteRenderer* renderer = builder.addSpriteRenderer();

                std::string name = "map/";
                name += std::to_string(x);
                name += std::to_string(y);
                name += ".png";

                TextureResource* textureResource = resources->get<TextureResource>(ResourceType::Texture, name);

               // glm::vec2 scale = glm::vec2(1920.0f / float(textureResource->width), 1080.0f / float(textureResource->height));

                Transform* transform = builder.addTransformComponent();

				transform->m_position = glm::vec2(x * 1920, y * 1080 );
                //transform->m_scale = scale;
                renderer->setTexture(textureResource);

                renderer->anchor();
            }
        }
	}

	
	void SandboxScene::createGladiators(GameCreateGladiatorsPacket* packet)
	{
		// This differs from updatePacket by setting the ownerId.
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
		}

	}
	void SandboxScene::createPlatform(GamePlatformPacket* packet)
	{
		ArenaPlatform platform;
		platform.type = (ArenaPlatformType)packet->m_platform.m_type;
		for (unsigned i = 0; i < packet->m_platform.m_vertexAmount; i++)
		{
			platform.vertices.push_back(packet->m_platform.m_vertexArray[i]);
		}
		// TODO: Make proper physics platform drawing for debugging.
		// This draw only draws sprites.
		/*
		EntityBuilder builder;
		builder.begin();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = platform.vertices[0];
		

		ResourceManager* resources = App::instance().resources();
        (void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();
		
		
		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "perkele.png"));
		
		renderer->anchor();
		*/
		m_platformVector.push_back(platform);

	}
	void SandboxScene::updateGladiators(GameUpdatePacket* packet)
	{
		for (int32_t i = 0; i < packet->m_playerAmount; i++)
		{
			uint8_t playerId = packet->m_characterArray[i].m_ownerId;
			*m_clientIdToGladiatorData[playerId]->m_gladiator->m_position = packet->m_characterArray[i].m_position;
			m_clientIdToGladiatorData[playerId]->m_transform->m_position= glm::vec2(packet->m_characterArray[i].m_position.x, packet->m_characterArray[i].m_position.y-64.0f);
			*m_clientIdToGladiatorData[playerId]->m_gladiator->m_velocity = packet->m_characterArray[i].m_velocity;
			m_clientIdToGladiatorData[playerId]->m_gladiator->m_aimAngle = packet->m_characterArray[i].m_aimAngle;
			//printf("Received update on gladiator position: %f, %f \n", packet->m_characterArray[i].m_position.x, packet->m_characterArray[i].m_position.y);
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
		}

	}
	void SandboxScene::createBullet(BulletData &data)
	{
		Bullet* bullet = new Bullet;
		*bullet->m_position = data.m_position;
		bullet->m_bulletId = data.m_id;
		bullet->m_type = (BulletType)data.m_type;
		bullet->m_creationDelay = data.m_creationDelay;
		bullet->m_rotation = data.m_rotation;

		createBulletEntity(bullet);
	}

	void SandboxScene::createBulletEntity(Bullet* bullet)
	{
		int spriteX = 0, spriteY = 0;
		EntityBuilder builder;
		builder.begin();

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
		
		Entity* entity = builder.getResults();
		registerEntity(entity);
		
		builder.begin();
		DebugBullet debugBullet;
		debugBullet.bullet = bullet;
		debugBullet.entity = entity;
		m_debugBullets.insert(std::pair<uint8_t, DebugBullet>(debugBullet.bullet->m_bulletId, debugBullet));

		// effects
		// Load muzzle flash, set rotation and position on spawn. Delete flash when finished.
		builder.begin();

		transform = builder.addTransformComponent();
		renderer = builder.addSpriteRenderer();
		Timer* timer = builder.addTimer();
		timer->m_lifeTime = 0.05f;

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/muzzleFlash_ss.png"));
		Rectf& source = renderer->getSource();

		spriteX = rand() % 2;
		if (spriteX == 0) {
			spriteY = rand() % 2;
		}

		source.x = 0 + spriteX * 32; source.y = 0 + spriteY * 32; source.w = 32; source.h = 32;
		
		transform->m_position = glm::vec2(bullet->m_position->x - 16, bullet->m_position->y - 16);
		glm::vec2& origin = renderer->getOrigin();
		origin.x = origin.x + 16; origin.y = origin.y + 16;
		renderer->setRotation(bullet->m_rotation + 3.142);
		renderer->setLayer(2);

		registerEntity(builder.getResults());
		// muzzle flash end.

		// Load gun smoke, randomize rotation and position and transparency on spawn.
		
		
		//

		for (int i = 0; i < rand() % 5 + 3; i++) 
		{
			spriteX = rand() % 4;
			float rotation = 0;
			int xOffset = 0, yOffset = 0;

			builder.begin();

			// Timer
			timer = builder.addTimer();
			timer->m_lifeTime = 1.0f;

			// Drawing stuff
			transform = builder.addTransformComponent();
			renderer = builder.addSpriteRenderer();
		
			renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/gunSmoke1_ss.png"));
			uint32_t color = color::toABGR(255, 255, 255, 125);
			renderer->setColor(color);
			renderer->setLayer(1);
			//renderer->setRotation(rand() % 7);
			Rectf& source = renderer->getSource();
			source.x = 32 * spriteX; source.y = 0; source.w = 32; source.h = 32;

			
			if (rand() % 2 == 1) {
				xOffset = rand() % 10;
				rotation = (rand() % 3)/100.0f;
			} else {
				xOffset = -(rand() % 10);
				rotation = -(rand() % 3)/100;
			}
			if (rand() % 2 == 1) {
				yOffset = rand() % 10;
			}
			else {
				yOffset = -(rand() % 10);
			}

			transform->m_position = glm::vec2(bullet->m_position->x+xOffset-16, bullet->m_position->y+yOffset-16);
			
			// Movement
			Movement* movement = builder.addMovement();
			movement->m_velocity = glm::vec2(float(xOffset)/100.0f , float(yOffset) / 100.0f);
			movement->m_rotationSpeed = rotation;

			registerEntity(builder.getResults());
		} //gun smoke end

	}
	

	void SandboxScene::createBulletHit(BulletData& data)
	{
		Bullet bullet;
		*bullet.m_position = data.m_position;
		bullet.m_type = (BulletType)data.m_type;
		bullet.m_rotation = data.m_rotation;
		m_bulletHitVector.push_back(bullet);

		createBulletHitEntity(bullet);
	}

	void SandboxScene::createBulletHitEntity(Bullet& bullet)
	{
		EntityBuilder builder;
		builder.begin();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = *bullet.m_position;

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "bullet_placeholder3.png"));

		renderer->setColor(0);
		renderer->anchor();
		registerEntity(builder.getResults());
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

		createBulletHitEntity(bullet);

		

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
	}
	void SandboxScene::GameUpdateScoreBoard(GameUpdateScoreBoardPacket* packet)
	{
		m_scoreboard.m_flagHolder = packet->m_scoreBoardData.m_flagHolder;
		for (unsigned i = 0; i < packet->m_playerAmount; i++)
		{
			m_scoreboard.m_playerScoreVector[i].m_score = (packet->m_scoreBoardData.m_playerScoreArray[i].m_score);
			m_scoreboard.m_playerScoreVector[i].m_tickets = (packet->m_scoreBoardData.m_playerScoreArray[i].m_tickets);
		}
	}
	
	void SandboxScene::sendInput(PlayerController &controller)
	{
        GameInputPacket* packet = (GameInputPacket*)createPacket(PacketTypes::GameInput);
		packet->m_aimAngle = controller.aimAngle;
        packet->m_input = controller.m_input;

		packet->m_clientSalt = s_client->m_clientSalt;
        packet->m_challengeSalt = s_client->m_challengeSalt;

		s_client->sendPacketToServer(packet, s_stamp);
	}

}