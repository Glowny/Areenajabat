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
#include <common\packet.h>


#include "common/arena/game_map.h"
#include "common\arena\weapons.h"
#include "common\arena\gladiator.h"


namespace arena
{
	static double s_stamp = 0.0;
	static NetworkClient* s_client;

	static Entity* entity;
	static Animator* s_animator;

	static void left(const void*)
	{
		s_animator->m_animator.setFlipX(false);
	}

	static void right(const void*)
	{
		s_animator->m_animator.setFlipX(true);
	}

	static void connect(const void*)
	{
		if (s_client->isConnected()) return;
		if (s_client->isConnecting()) return;

		s_client->connect("localhost", uint16_t(8088), s_stamp);
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
				switch (packet->getType())
				{
				case PacketTypes::GameSetup:
				{
					GameSetupPacket* setupPacket = (GameSetupPacket*)packet;
					m_playerId = setupPacket->m_clientSalt;
					createGladiators(setupPacket->m_playerAmount);
					break;
				}
				case PacketTypes::GameUpdate:
				{
					GameUpdatePacket* updatePacket = (GameUpdatePacket*)packet;
					updateGladiators(updatePacket);
					break;
				}
				case PacketTypes::GamePlatform:
				{
					GamePlaformPacket* platformPacket = (GamePlaformPacket*)packet;
					createPlatform(platformPacket);
					break;
				}
				case PacketTypes::GameSpawnBullets:
				{
					GameSpawnBulletsPacket* bulletPacket = (GameSpawnBulletsPacket*)packet;
					spawnBullets(bulletPacket);
					break;
				}
				case PacketTypes::GameBulletHit:
				{

					break;
				}
				case PacketTypes::GameDamagePlayer:
				{

					break;
				}
				case PacketTypes::GameKillPlayer:
				{

					break;
				}
				case PacketTypes::GameRespawnPlayer:
				{

					break;
				}
				case PacketTypes::GameUpdateScoreBoard:
				{

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
		auto tx = (Transform* const)entity->first(TYPEOF(Transform));

		Camera& camera = App::instance().camera();
		camera.m_position = tx->m_position;
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

		glm::vec2 dir(mouseLoc - tx->m_position);
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
		EntityBuilder builder;

		builder.begin();

		ResourceManager* resources = App::instance().resources();

		SpriteRenderer* renderer = builder.addSpriteRenderer();
		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "perkele.png"));
		renderer->anchor();

		Transform* transform = builder.addTransformComponent();
		transform->m_position.x = 500.f;
		transform->m_position.y = 500.0f;


		s_animator = builder.addCharacterAnimator();
		CharacterAnimator& anim = s_animator->m_animator;
		anim.setStaticContent(
			resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Crest.png"),
			resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Helmet.png"),
			resources->get<TextureResource>(ResourceType::Texture, "Characters/body/1_Torso.png"),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/LegAnimations/Run.scml")->getNewEntityInstance(0)
		);
		anim.setWeaponAnimation(WeaponAnimationType::Gladius);
		//anim.setPosition(glm::vec2(0, 0));

		entity = builder.getResults();

		registerEntity(entity);

		inputAddBindings("player", s_bindings);
	}
	void SandboxScene::onDestroy()
	{
		inputRemoveBindings("player");
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
		m_gladiatorVector[packet->m_targetID]->m_hitpoints -= packet->m_damageAmount;
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
	void SandboxScene::GameUpdateScoreBoard(GameUpdateScoreBoardPacket packet)
	{


	}
}