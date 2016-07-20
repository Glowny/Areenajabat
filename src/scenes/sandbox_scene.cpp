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

#ifdef _DEBUG
#	include <iostream>
#   include "..\rtti\rtti_define.h"
#   include "..\ecs\transform.h"
#   include "..\ecs\sprite_renderer.h"
#	include "../res/texture_resource.h"
#endif

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
    
    static void connect(const void*)
    {
        if (s_client->isConnected()) return;
        if (s_client->isConnecting()) return;

        s_client->connect("localhost", uint16_t(8088), s_stamp);
        //s_client->createLobby("perkele", s_stamp);
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
		{ arena::Key::KeyH, arena::Modifier::None, 0, inputMoveLeft, "moveleft" },
		{ arena::Key::KeyK, arena::Modifier::None, 0, inputMoveRight, "moveright" },
		{ arena::Key::KeyU, arena::Modifier::None, 0, inputMoveUp, "moveup" },
		{ arena::Key::KeyY, arena::Modifier::None, 0, inputShoot, "shoot" },
        { arena::Key::KeyQ, arena::Modifier::None, 0, connect, "connect" },
        { arena::Key::KeyE, arena::Modifier::None, 0, disconnect, "disconnect" },
        INPUT_BINDING_END
    };

	static void inputMoveLeft(const void*)
	{
        sandbox->m_controller.m_input.m_leftButtonDown = true;
        sandbox->m_controller.moveFlag = true;
	}
	static void inputMoveRight(const void*)
	{
        sandbox->m_controller.m_input.m_rightButtonDown = true;
        sandbox->m_controller.moveFlag = true;
	}
	static void inputMoveUp(const void*)
	{
        sandbox->m_controller.m_input.m_upButtonDown = true;
        sandbox->m_controller.moveFlag = true;
	}
	static void inputShoot(const void*)
	{
        sandbox->m_controller.m_input.m_shootButtonDown = true;
        sandbox->m_controller.shootFlag = true;
	}

	void SandboxScene::setAimAngle(float angle)
	{
		m_controller.aimAngle = angle;
		m_controller.moveFlag = true;
	}

	void SandboxScene::setShoot()
	{
		m_controller.shootFlag = true;
	}

	SandboxScene::SandboxScene() : Scene("sandbox")
	{
		sandbox = this;
		sendInputToServerTimer = 0;
		m_controller.aimAngle = 0;
		m_controller.moveFlag = false;
		m_controller.m_jumpDirection = 0;
		m_controller.weapon = Gladius;
		m_controller.shootFlag = false;
		createBackground();
	}

    void SandboxScene::onUpdate(const GameTime& gameTime)
    {
		s_stamp = gameTime.m_total;

        s_client->sendMatchMakingPackets(gameTime.m_total);
        s_client->sendProtocolPackets(gameTime.m_total);
		
		if ((sendInputToServerTimer += gameTime.m_delta) > 0.016f)
		{
			if(m_controller.moveFlag == true)
			{ 
				sendInput(m_controller);
				m_controller.moveFlag = false;
                // reset
                memset(&m_controller.m_input, false, sizeof(PlayerInput));
			}
			if (m_controller.shootFlag == true)
			{
				sendShootEvent(m_controller.aimAngle);
				m_controller.shootFlag = false;
			}
			
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
                    GameSetupPacket* setupPacket = (GameSetupPacket*)packet;
                    m_playerId = setupPacket->m_clientSalt;
                    createGladiators(setupPacket->m_playerAmount);
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
                    damagePlayer((GameDamagePlayerPacket*)packet);
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
		if (m_gladiatorDrawDataVector.size() != 0)
		{ 
			Transform* playerTransform = (Transform* const)m_gladiatorDrawDataVector[0].m_entity->first(TYPEOF(Transform));
			
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

			bgfx::dbgTextPrintf(0, 1, 0x9f, "Delta time %.10f", gameTime.m_delta);
			bgfx::dbgTextPrintf(0, 2, 0x8f, "Left btn = %s, Middle btn = %s, Right btn = %s",
			    mouse.m_buttons[MouseButton::Left] ? "down" : "up",
			    mouse.m_buttons[MouseButton::Middle] ? "down" : "up",
			    mouse.m_buttons[MouseButton::Right] ? "down" : "up");
			bgfx::dbgTextPrintf(0, 3, 0x6f, "Mouse (screen) x = %d, y = %d, wheel = %d", mouse.m_mx, mouse.m_my, mouse.m_mz);
			bgfx::dbgTextPrintf(0, 4, 0x9f, "Mouse pos (world) x= %.2f, y=%.2f", mouseLoc.x, mouseLoc.y);
			bgfx::dbgTextPrintf(0, 5, 0x9f, "Angle (%.3f rad) (%.2f deg)", a, glm::degrees(a));

			App::instance().spriteBatch()->submit(0);
		}
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
	
	void SandboxScene::createGladiator(glm::vec2 position)
	{
		Entity* gladiator;
		EntityBuilder builder;
		builder.begin();

		ResourceManager* resources = App::instance().resources();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = position;

		Animator* animator = builder.addCharacterAnimator();
		CharacterAnimator& anim = animator->m_animator;
		anim.setStaticContent(
			resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Crest.png"),
			resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Helmet.png"),
			resources->get<TextureResource>(ResourceType::Texture, "Characters/body/1_Torso.png"),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/LegAnimations/Run.scml")->getNewEntityInstance(0)
		);
		anim.setWeaponAnimation(WeaponAnimationType::Gladius);
		//anim.setPosition(glm::vec2(0, 0));

		gladiator = builder.getResults();

		registerEntity(gladiator);

		// TODO: should use entities.
		GladiatorDrawData data;
		data.m_entity = gladiator;
		data.m_animator = animator;
		data.m_transform = transform;
		data.m_gladiator = Gladiator();
		m_gladiatorDrawDataVector.push_back(data);

		// TODO: do animation stuff better
		anime = m_gladiatorDrawDataVector[m_playerId].m_animator;
	}
	void SandboxScene::createBackground()
	{
		
		for(unsigned y = 0; y <= 1; y++)
		{ 
		for(unsigned x = 0; x <= 3; x++)
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

				glm::vec2 scale = glm::vec2(1920.0f / float(textureResource->width), 1080.0f / float(textureResource->height));
				
				Transform* transform = builder.addTransformComponent();

				transform->m_position = glm::vec2(x * 1920, y * 1080 +64);
				transform->m_scale = scale;
				renderer->setTexture(textureResource);

				renderer->anchor();
			}
		}
	}

	
	void SandboxScene::createGladiators(unsigned amount)
	{
		for (unsigned i = 0; i < amount; i++)
		{
			Gladiator* gladiator = new Gladiator();
			gladiator->m_alive = true;
			gladiator->m_hitpoints = 100;
			gladiator->m_position = glm::vec2(0, 0);
			gladiator->m_rotation = 0;
			Weapon* weapon = new WeaponGladius();
			gladiator->m_weapon = weapon;
			createGladiator(gladiator->m_position);
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
		printf("platformVector size: %d\n", m_platformVector.size());
	}
	void SandboxScene::updateGladiators(GameUpdatePacket* packet)
	{
		for (unsigned i = 0; i < packet->m_playerAmount; i++)
		{
			// TODO: SET CORRECT ID TO SYNCH
			unsigned index = 0;// packet->m_characterArray[i].m_ownerId;
			m_gladiatorDrawDataVector[index].m_gladiator.m_position = packet->m_characterArray[i].m_position;
			m_gladiatorDrawDataVector[index].m_transform->m_position= packet->m_characterArray[i].m_position;
			m_gladiatorDrawDataVector[index].m_gladiator.m_velocity = packet->m_characterArray[i].m_velocity;
			m_gladiatorDrawDataVector[index].m_gladiator.m_rotation = packet->m_characterArray[i].m_rotation;
			//printf("Received update on gladiator position: %f, %f \n", packet->m_characterArray[i].m_position.x, packet->m_characterArray[i].m_position.y);
		}
		
	}
	void SandboxScene::spawnBullets(GameSpawnBulletsPacket* packet)
	{
		for (unsigned i = 0; i < packet->m_bulletAmount; i++)
		{
			Bullet bullet;
			bullet.m_position = packet->m_bulletSpawnArray[i].m_position;
			bullet.m_type = (BulletType)packet->m_bulletSpawnArray[i].m_type;
			bullet.m_creationDelay = packet->m_bulletSpawnArray[i].m_creationDelay;
			bullet.m_rotation = packet->m_bulletSpawnArray[i].m_rotation;
			m_spawnBulletVector.push_back(bullet);
		}
	}
	void SandboxScene::spawnBulletHits(GameBulletHitPacket *packet)
	{
		for (unsigned i = 0; i < packet->m_bulletAmount; i++)
		{
			Bullet bullet;
			bullet.m_position = packet->bulletHitArray[i].m_position;
			bullet.m_type = (BulletType)packet->bulletHitArray[i].m_type;
			bullet.m_rotation = packet->bulletHitArray[i].m_rotation;
			m_bulletHitVector.push_back(bullet);
		}

	}
	// These might not work
	void SandboxScene::damagePlayer(GameDamagePlayerPacket* packet)
	{
		m_gladiatorDrawDataVector[packet->m_targetID].m_gladiator.m_hitpoints -= int32(packet->m_damageAmount);
	}
	void SandboxScene::killPlayer(GameKillPlayerPacket* packet)
	{
		m_gladiatorDrawDataVector[packet->m_playerID].m_gladiator.m_hitpoints = 0;
		m_gladiatorDrawDataVector[packet->m_playerID].m_gladiator.m_alive = false;
	}
	void SandboxScene::respawnPlayer(GameRespawnPlayerPacket* packet)
	{
		m_gladiatorDrawDataVector[packet->m_playerID].m_gladiator.m_hitpoints = 100;
		m_gladiatorDrawDataVector[packet->m_playerID].m_gladiator.m_alive = true;
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
	void SandboxScene::sendShootEvent(float angle)
	{
		GameShootPacket* packet = (GameShootPacket*)createPacket(PacketTypes::GameShoot);
		packet->m_angle = angle;
        packet->m_clientSalt = s_client->m_clientSalt;
        packet->m_challengeSalt = s_client->m_challengeSalt;

		s_client->sendPacketToServer(packet, s_stamp);
	}
}