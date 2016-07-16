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

namespace arena
{
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
        { arena::Key::KeyQ, arena::Modifier::None, 0, connect, "connect" },
        { arena::Key::KeyE, arena::Modifier::None, 0, disconnect, "disconnect" },
        INPUT_BINDING_END
    };


	SandboxScene::SandboxScene() : Scene("sandbox")
	{
	}

    void SandboxScene::onUpdate(const GameTime& gameTime)
    {
        s_stamp = gameTime.m_total;

        s_client->sendMatchMakingPackets(gameTime.m_total);
        s_client->sendProtocolPackets(gameTime.m_total);

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
                    for (unsigned i = 0; i < setupPacket->m_playerAmount; i++)
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
                    createPlatform((GamePlaformPacket*)packet);
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
                    printf("Unknown packet type received on sandbox_scene\n");
                }
                }
            }

            destroyPacket(packet);
        }
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

	void SandboxScene::onInitialize()
	{
        s_client = new NetworkClient(uint16_t(13337));
        s_client->m_lobbyListener = &s_lobbyListener;
		m_playerId = 0;
		for (unsigned i = 0; i < 3; i++)
		{
			createGladiator(glm::vec2(100*i, 300));
		}
		anime = m_gladiatorDrawDataVector[m_playerId].m_animator;
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
		GladiatorDrawData data;
		data.m_entity = gladiator;
		data.m_animator = animator;
		m_gladiatorDrawDataVector.push_back(data);
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
	void SandboxScene::createPlatform(GamePlaformPacket* packet)
	{
		ArenaPlatform platform;
		platform.type = (ArenaPlatformType)packet->m_platform.m_type;
		for (unsigned i = 0; i < packet->m_platform.m_vertexAmount; i++)
		{
			platform.vertices.push_back(packet->m_platform.m_vertexArray[i]);
		}

		EntityBuilder builder;
		builder.begin();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = platform.vertices[0];

		ResourceManager* resources = App::instance().resources();

		SpriteRenderer* renderer = builder.addSpriteRenderer();
		
		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "perkele.png"));
		
		renderer->anchor();

		m_platformVector.push_back(platform);
	}
	void SandboxScene::updateGladiators(GameUpdatePacket* packet)
	{
		if (m_gladiatorVector.size() != packet->m_playerAmount)
			printf("DIFFERENT AMOUNT OF GLADIATORS ON UPDATEPACKAGE\n");
		for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
		{
			m_gladiatorVector[i]->m_position = packet->m_characterArray[i].m_position;
			m_gladiatorVector[i]->m_velocity = packet->m_characterArray[i].m_velocity;
			m_gladiatorVector[i]->m_rotation = packet->m_characterArray[i].m_rotation;
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
	void SandboxScene::damagePlayer(GameDamagePlayerPacket* packet)
	{
		m_gladiatorVector[packet->m_targetID]->m_hitpoints -= int32(packet->m_damageAmount);
	}
	void SandboxScene::killPlayer(GameKillPlayerPacket* packet)
	{
		m_gladiatorVector[packet->m_playerID]->m_hitpoints = 0;
		m_gladiatorVector[packet->m_playerID]->m_alive = false;
	}
	void SandboxScene::respawnPlayer(GameRespawnPlayerPacket* packet)
	{
		m_gladiatorVector[packet->m_playerID]->m_hitpoints = 100;
		m_gladiatorVector[packet->m_playerID]->m_alive = true;
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
		GameInputPacket* packet = new GameInputPacket();
		packet->m_aimAngle = controller.aimAngle;
		packet->x = controller.m_movementDirection.x;
		packet->y = controller.m_movementDirection.y;
		packet->m_clientSalt = s_client->m_clientSalt;
        packet->m_challengeSalt = s_client->m_challengeSalt;

		s_client->sendPacketToServer(packet, s_stamp);
	}
	void SandboxScene::sendShootEvent(float angle)
	{
		GameShootPacket* packet = new GameShootPacket;
		packet->m_angle = angle;
        packet->m_clientSalt = s_client->m_clientSalt;
        packet->m_challengeSalt = s_client->m_challengeSalt;

		s_client->sendPacketToServer(packet, s_stamp);
	}
}