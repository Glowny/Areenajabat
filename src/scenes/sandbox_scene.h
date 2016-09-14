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
#include <common/arena/physics.h>
#include <common/arena/game_mode_factory.h>
#include "src/ecs/entity_factory.h"

struct Message;


namespace arena
{
	class Animator;
	struct Weapon;
	struct Bullet;
	struct PlayerScore;
	
	class SandboxScene final : public Scene 
	{
	public:
		SandboxScene();
		~SandboxScene() = default;
        PlayerController m_controller;
		bool m_toggleKeyBindDraw;
		bool m_toggleScoreboardDraw = false;
		bool m_toggleGameStateDraw = false;
		void requestMap(uint8_t mapID);
		bool gameRunning;
		bool hasMap;
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
		void setGameMode(GameModePacket* packet);
		
		void cleanUp();

		// Update all entities on ecs.
		void updateEntities(const GameTime& gameTime);

		// Update clientside physics
		void updatePhysics(float32 timeStep);

		void updateServerBullets(const GameTime& gameTime);

		// Create bullets shot by other players
		void createBullet(BulletData& data);
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

		// Platform data that is send by server and can be used for clientside physics.
		std::vector<ArenaPlatform> m_platformVector; 

		float64 m_sendInputToServerTimer;

		// m_playerId is used to see which gladiator player is controlling.
		uint8_t m_playerId; 

		// m_backgroundSetting is used to set which backgrounds are loaded.
		// 0 = no background and no foreground, 1 = foreground, 2 = background, 3 = foreground and background
		int m_backgroundSetting = 1; 
		Entity* mousePointerEntity;
		Physics m_physics;

		//TODO: remake as component later.
		glm::vec2 oldPlayerPos = glm::vec2(0, 0);
		Scoreboard* m_scoreboard;
		GameMode* m_gameMode;
		glm::vec2 m_screenSize = glm::vec2(1920, 1080);

		glm::vec2 m_weaponRotationPoint;

		bool m_physicsUpdated = false;
		bool debugBullets = false;
		glm::ivec2 m_mouseValues{ 0,0 };
		EntityFactory* m_factory;
		std::vector<Player> m_players;
    };

	static void inputMoveLeft(const void*);
	static void inputMoveRight(const void*);
	static void inputMoveUp(const void*);
	static void inputMoveDown(const void*);
	static void inputReload(const void*);
	static void inputJump(const void*);
	static void toggleKeyBindDraw(const void*);
	static void toggleScoreBoardDraw(const void*);

}