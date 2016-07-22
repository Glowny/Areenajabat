#pragma once

#include "scene.h"
#include <queue>
#include <vector>
#include "common\arena\game_map.h"
#include "common\arena\playerController.h"
#include <common\arena\arena_packet.h>
#include "common\arena\scoreboard.h"
#include "..\ecs\transform.h"
#include "common\arena\gladiator.h"
#include <map>
struct Message;


namespace arena
{
	class Animator;

	struct DebugBullet
	{
		DebugBullet()
		{
			lifeTime = 2.0f;
		}
		Entity* entity;
		Bullet* bullet;
		float lifeTime;
	};
	struct Weapon;
	struct Bullet;
	struct PlayerScore;
	struct GladiatorDrawData
	{
		GladiatorDrawData()
		{
			m_entity = NULL;
			m_transform = NULL;
			m_animator = NULL;
			m_gladiator = NULL;
		}
		Entity* m_entity;
		Transform* m_transform;
		Animator* m_animator;
		Gladiator* m_gladiator;
	};
	
	class SandboxScene final : public Scene 
	{
	public:
		SandboxScene();
		~SandboxScene() = default;
		
		void setShoot();
		void setAimAngle(float angle);
	protected:
		virtual void onUpdate(const GameTime& time) final override;
		virtual void onInitialize() final override;
		virtual void onDestroy() final override;
	private:

		void createBackground();
		void createGladiators(GameCreateGladiatorsPacket* packet);
		void createPlatform(GamePlatformPacket* packet);
		void updateGladiators(GameUpdatePacket* packet);
		void spawnBullets(GameSpawnBulletsPacket* packet);
		void spawnBulletHits(GameBulletHitPacket* packet);
		void damagePlayer(GameDamagePlayerPacket* packet);
		void killPlayer(GameKillPlayerPacket* packet);
		void respawnPlayer(GameRespawnPlayerPacket* packet);
		void GameUpdateScoreBoard(GameUpdateScoreBoardPacket* packet);

		void sendInput(PlayerController &controller);

		void createGladiator(Gladiator* gladiator);

		// Create bullets shot by other players
		void createBullet(BulletData& data);
		void createBulletEntity(Bullet* bullet);

		// Create real hits (debugging & adjusting)
		void createBulletHit(BulletData& data);
		void createBulletHitEntity(Bullet& bullet);

		// Create fake bullets when player shoots.
		void createClientSideBullet(Bullet bullet);
		// Create fake hits when bullets hit.
		void createClientSideHit(Bullet bullet);

		// Fake death or not?
		void clientSideGladiatorDeath(unsigned id);

		std::map<uint8_t, GladiatorDrawData*> m_clientIdToGladiatorData;
	
		// TODO: should use entities

		std::vector<ArenaPlatform> m_platformVector;
		std::vector<Bullet> m_bulletHitVector;

		std::map<uint8_t, DebugBullet> m_debugBullets;
		bool connected;

		float sendInputToServerTimer;
		Scoreboard m_scoreboard;
		uint8_t m_playerId;

    public:
        PlayerController m_controller;
	};

	static void inputMoveLeft(const void*);
	static void inputMoveRight(const void*);
	static void inputMoveUp(const void*);
	static void inputShoot(const void*);
}