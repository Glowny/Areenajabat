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
struct Message;


namespace arena
{
	class Animator;


	struct Weapon;
	struct Bullet;
	struct PlayerScore;
	struct GladiatorDrawData
	{
		Entity* m_entity;
		Transform* m_transform;
		Animator* m_animator;
		Gladiator m_gladiator;
	};
	
	class SandboxScene final : public Scene 
	{
	public:
		SandboxScene();
		~SandboxScene() = default;
		void setInput(glm::ivec2 direction);
		void setShoot();
		void setAimAngle(float angle);
	protected:
		virtual void onUpdate(const GameTime& time) final override;
		virtual void onInitialize() final override;
		virtual void onDestroy() final override;
	private:

		void createGladiators(unsigned amount);
		void createPlatform(GamePlatformPacket* packet);
		void updateGladiators(GameUpdatePacket* packet);
		void spawnBullets(GameSpawnBulletsPacket* packet);
		void spawnBulletHits(GameBulletHitPacket* packet);
		void damagePlayer(GameDamagePlayerPacket* packet);
		void killPlayer(GameKillPlayerPacket* packet);
		void respawnPlayer(GameRespawnPlayerPacket* packet);
		void GameUpdateScoreBoard(GameUpdateScoreBoardPacket* packet);

		void sendShootEvent(float angle);
		void sendInput(PlayerController &controller);

		void createGladiator(glm::vec2 position);

		// Create bullets shot by other players
		void createBullet(Bullet bullet);

		// Create real hits (debugging & adjusting)
		void createHit(Bullet bullet);

		// Create fake bullets when player shoots.
		void createClientSideBullet(Bullet bullet);
		// Create fake hits when bullets hit.
		void createClientSideHit(Bullet bullet);

		// Fake death or not?
		void clientSideGladiatorDeath(unsigned id);

		std::vector<GladiatorDrawData> m_gladiatorDrawDataVector;
		// TODO: should use entities

		std::vector<ArenaPlatform> m_platformVector;
		std::vector<Bullet> m_spawnBulletVector;
		std::vector<Bullet> m_bulletHitVector;

		PlayerController m_controller;
		float sendInputToServerTimer;
		Scoreboard m_scoreboard;
		uint64 m_playerId;
	};

	static void inputMoveLeft(const void*);
	static void inputMoveRight(const void*);
	static void inputMoveUp(const void*);
	static void inputShoot(const void*);
}