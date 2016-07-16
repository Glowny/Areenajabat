#pragma once

#include "scene.h"
#include <queue>
#include <vector>
#include "common\arena\game_map.h"
#include "common\arena\playerController.h"
#include <common\arena\arena_packet.h>
#include "common\arena\scoreboard.h"
struct Message;


namespace arena
{
	struct Animator;
	struct GladiatorDrawData
	{
		Entity* m_entity;
		Animator* m_animator;

	};

	struct Weapon;
	struct Bullet;
	struct Gladiator;
	struct PlayerScore;

	class SandboxScene final : public Scene 
	{
	public:
		SandboxScene();
		~SandboxScene() = default;
	protected:
		virtual void onUpdate(const GameTime& time) final override;
		virtual void onInitialize() final override;
		virtual void onDestroy() final override;
	private:

		void createGladiators(unsigned amount);
		void createPlatform(GamePlaformPacket* packet);
		void updateGladiators(GameUpdatePacket* packet);
		void spawnBullets(GameSpawnBulletsPacket* packet);
		void spawnBulletHits(GameBulletHitPacket* packet);
		void damagePlayer(GameDamagePlayerPacket* packet);
		void killPlayer(GameKillPlayerPacket* packet);
		void respawnPlayer(GameRespawnPlayerPacket* packet);
		void GameUpdateScoreBoard(GameUpdateScoreBoardPacket* packet);

		void sendInput(PlayerController &controller);
		void sendShootEvent(float angle);

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
		std::vector<Gladiator*> m_gladiatorVector;
		std::vector<ArenaPlatform> m_platformVector;
		std::vector<Bullet> m_spawnBulletVector;
		std::vector<Bullet> m_bulletHitVector;

		PlayerController m_controller;
		Scoreboard m_scoreboard;
		uint64 m_playerId;
	};
}