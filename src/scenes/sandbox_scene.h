#pragma once

#include "scene.h"
#include <queue>
#include <vector>
#include <common/arena/arena_packet.h>
#include "common/arena/game_map.h"
#include "common/arena/playerController.h"
#include "common/arena/gladiator.h"
#include "common/arena/scoreboard.h"
#include "../ecs/transform.h"
#include <map>
struct Message;


namespace arena
{
	class Animator;
	struct DebugBullet
	{
		DebugBullet()
		{
			lifeTime = 0.0f;
		}
		~DebugBullet()
		{
			
		}
		void destroy()
		{
			delete bullet;
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
		void destroy();
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
        PlayerController m_controller;
	
	protected:
		virtual void onUpdate(const GameTime& time) final override;
		virtual void onInitialize() final override;
		virtual void onDestroy() final override;

	private:
		void sendInput(PlayerController &controller);
		void processAllPackets(const GameTime& gameTime);
		void processPacket(Packet* packet);

		void createGladiators(GameCreateGladiatorsPacket* packet);
		void createPlatform(GamePlatformPacket* packet);
		void updateGladiators(GameUpdatePacket* packet);
		void spawnBullets(GameSpawnBulletsPacket* packet);
		void spawnBulletHits(GameBulletHitPacket* packet);
		void processDamagePlayer(GameDamagePlayerPacket* packet);
		void killPlayer(GameKillPlayerPacket* packet);
		void respawnPlayer(GameRespawnPlayerPacket* packet);
		void updateScoreBoard(GameUpdateScoreBoardPacket* packet);
		
		// Update all entities on ecs.
		void updateEntities(const GameTime& gameTime);
		// Update debugbullets.
		void updateDebugBullets(const GameTime& gameTime);

		Entity* createMousePointerEntity();
		// Create single gladiator.
		void createGladiator(Gladiator* gladiator);
		// Create bullets shot by other players
		void createBullet(BulletData& data);
		void createBulletEntity(Bullet* bullet);
		void createMuzzleFlashEntity(const Bullet& bullet);
		void createSmokeEntity(const Bullet& bullet);

		void createBulletHit(BulletHitData& data);
		void createBloodBulletHitEntity(Bullet& bullet);
		void createPlatformBulletHitEntity(Bullet& bullet);
		void destroyBullet(uint8_t bulletId);
		// Update camera position to player gladiator position.
		void updateCameraPosition(); 
		// Rotata player aim according to mouse position.
		void rotatePlayerAim();
		// Draw debug text.
		void setDrawText(const GameTime& gameTime); 

		// Load background from file.
		void createBackground(); 
		void checkBounds(glm::vec2& cameraPosition);
		// TODO: should use entities
		std::map<uint8_t, GladiatorDrawData*> m_clientIdToGladiatorData;
		// Platform data that is send by server and can be used for clientside physics.
		std::vector<ArenaPlatform> m_platformVector; 

		std::map<uint8_t, DebugBullet> m_debugBullets;

		float m_sendInputToServerTimer;

		// m_playerId is used to see which gladiator player is controlling.
		uint8_t m_playerId; 
		// m_nextSprite is used to choose the next sprite in muzzle flash spritesheet. TODO: should be gladiator specific
		int m_nextSprite = 0; 
		// m_backgroundSetting is used to set which backgrounds are loaded.
		// 0 = no background and no foreground, 1 = foreground, 2 = background, 3 = foreground and background
		int m_backgroundSetting; 
		Entity* mousePointerEntity;
		//TODO: remake as component later.
		glm::vec2 oldMousePos;
		Scoreboard* m_scoreboard;
		glm::vec2 m_screenSize = glm::vec2(1920, 1080);
    };

	static void inputMoveLeft(const void*);
	static void inputMoveRight(const void*);
	static void inputMoveUp(const void*);
	static void inputMoveDown(const void*);
	static void inputShoot(const void*);
	static void inputReload(const void*);
	static void inputJump(const void*);

}