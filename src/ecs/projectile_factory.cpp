#include "projectile_factory.h"
#include "entity_builder.h"
#include "transform.h"
#include <cassert>
#include <common/debug.h>
glm::vec2 radToVec(float r)
{
	return glm::vec2(cos(r), sin(r));
}
namespace arena
{
	void ProjectileFactory::createProjectile(BulletData &data)
	{
		if (data.m_type == BulletType::GrenadeBullet)
			createGrenade(data);
		else
			createBullet(data);

	}
	void ProjectileFactory::createBullet(BulletData &data)
	{
		glm::vec2 vectorAngle = radToVec(data.m_rotation);
		glm::vec2 force(0.0f, 0.0f);
		switch(data.m_type)
		{ 
			case BulletType::GladiusBullet:
				force = glm::vec2(vectorAngle.x * GLADIUSIMPULSE, vectorAngle.y * GLADIUSIMPULSE);
				break;
			case BulletType::ShardBullet:
				force = glm::vec2(vectorAngle.x * SHARDIMPULSE, vectorAngle.y * SHARDIMPULSE);
				break;
			case BulletType::ShotgunBullet:
				force = glm::vec2(vectorAngle.x * SHOTGUNIMPULSE, vectorAngle.y * SHOTGUNIMPULSE);
				break;
		}
		EntityBuilder builder;
		builder.begin();
		Transform* transform = builder.addTransformComponent();
		m_physics->addBulletWithID(&transform->m_position, force, data.m_rotation, data.m_ownerId, data.m_id);
	}
	void ProjectileFactory::createGrenade(BulletData &data)
	{

	}
	void ProjectileFactory::createMuzzleFlash(glm::vec2 position, float aimRotation)
	{

	}
	void ProjectileFactory::createSmoke(glm::vec2 position, glm::vec2 velocity)
	{

	}
	void ProjectileFactory::createMagazine(glm::vec2 position, glm::vec2 force, bool flip)
	{

	}
	void ProjectileFactory::createBulletHitBlood(unsigned direction, glm::vec2 position)
	{

	}
	void ProjectileFactory::createExplosion(glm::vec2 position, float rotation)
	{

	}
	void ProjectileFactory::createExplosionBlood(glm::vec2 position)
	{

	}
	void ProjectileFactory::createMiniBomb(uint32_t playerIndex, float time)
	{

	}
	void ProjectileFactory::createBulletHit(BulletHitData& data)
	{

	}
}