#pragma once

#include <glm/glm.hpp>
#include "../forward_declare.h"
#include "../arena_types.h"
#include "../rect.h"
#include  <common\arena\arena_packet.h>
#include  <common\arena\physics.h>

// This creates different projectiles.
namespace arena
{

	class ProjectileFactory final
	{
	public:
		ProjectileFactory(Physics* physics):m_physics(physics) {}

		void createProjectile(BulletData &data);
		void createBullet(BulletData &data);
		void createGrenade(BulletData &data);
		void createMuzzleFlash(glm::vec2 position, float aimRotation);
		void createSmoke(glm::vec2 position, glm::vec2 velocity);
		void createMagazine(glm::vec2 position, glm::vec2 force, bool flip);
		void createBulletHitBlood(unsigned direction, glm::vec2 position);
		void createExplosion(glm::vec2 position, float rotation);
		void createExplosionBlood(glm::vec2 position);
		void createMiniBomb(uint32_t playerIndex, float time);
		void createBulletHit(BulletHitData& data);

	private:
		Physics* m_physics;
	};
}
