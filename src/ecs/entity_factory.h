#pragma once

#include <glm/glm.hpp>
#include "../forward_declare.h"
#include "../arena_types.h"
#include "../rect.h"
#include  <common\arena\arena_packet.h>
#include  <common\arena\physics.h>
#include "../scenes/scene.h"
#include "../res/resource_manager.h"
#include "../camera.h"
// This creates different projectiles.
namespace arena
{

	class EntityFactory final
	{
	public:
		EntityFactory(Physics* physics, Scene* scene);

		Entity* createProjectile(BulletData &data);
		Entity* createBullet(BulletData &data);
		Entity* createGrenade(BulletData &data);
		Entity* createMuzzleFlash(glm::vec2 position, float aimRotation);
		Entity* createSmoke(glm::vec2 position, glm::vec2 velocity);
		Entity* createMagazine(glm::vec2 position, glm::vec2 force, bool flip);
		Entity* createBulletHitBlood(unsigned direction, glm::vec2 position);
		Entity* createExplosion(glm::vec2 position, float rotation);
		Entity* createExplosionBlood(glm::vec2 position);
		Entity* createMiniBomb(uint32_t playerIndex, float time);
		Entity* createBulletHit(BulletHitData& data);
		Entity* createMousePointer(Camera& camera);
		Entity* createGladiator(CharacterData& characterData);
		Entity* createCamera();
		void freeEntityId(uint8_t id);
	private:
		Scene* m_scene;
		Physics* m_physics;
		ResourceManager* m_resources;
		uint8_t getFreeEntityId();
		void nextUint8_t(uint8_t& current);
		uint8_t currentFreeId = 0;
		uint8_t isIdFree[255];
	};
}
