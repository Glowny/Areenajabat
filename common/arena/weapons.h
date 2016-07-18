#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <..\common\network_entity.h>

#define GLADIUSIMPULSE 100.0f
#define SHOTGUNIMPULSE 100.0f

namespace arena
{
	enum WeaponType
	{
		Gladius,
		Shotgun,
	};

	enum BulletType
	{
		GladiusBullet,
		ShotgunBullet,
	};

	struct Bullet : public NetworkEntity
	{
		Bullet()
			: NetworkEntity(NetworkEntityType::Projectile)
		{
			m_type = BulletType(0);
			m_position = glm::vec2(0,0);
			m_impulse = glm::vec2(0, 0);
			m_rotation = 0;
			m_creationDelay = 0;
		}
		BulletType m_type;
		glm::vec2 m_position;
		glm::vec2 m_impulse;
		float m_rotation;
		float m_creationDelay;
	};

	struct Weapon : public NetworkEntity
	{
	public:
		WeaponType m_type;
		
		Weapon() : NetworkEntity(NetworkEntityType::Weapon)
		{
		}
		
		virtual std::vector<Bullet*> createBullets(float aimAngle, glm::vec2 position) 
		{ 
			aimAngle; 
			position; 
			std::vector<Bullet*> temp; 
			return temp; 
		}
	protected:
		glm::vec2 radToVec(float r)
		{
			return glm::vec2(cos(r), sin(r));
		}
	};

	struct WeaponGladius : public Weapon
	{
		WeaponGladius() : Weapon() { m_type = Gladius; }

		std::vector<Bullet*> createBullets(float aimAngle, glm::vec2 position)
		{
			std::vector<Bullet*> bullets;
			for (unsigned i = 0; i < 4; i++)
			{
				glm::vec2 vectorAngle = radToVec(aimAngle);
				Bullet* bullet = new Bullet;
				bullet->m_type = GladiusBullet;
				bullet->m_creationDelay = 0.1f * i;
				bullet->m_rotation = aimAngle; // not really needed
				bullet->m_impulse.x = vectorAngle.x * 10; bullet->m_impulse.y = vectorAngle.y * 10;
				// Shotgun does not know playerposition, has to be moved +playerposition.x &.y later
				bullet->m_position.x = position.x + vectorAngle.x * 72; bullet->m_position.y = position.y + vectorAngle.y * 72;
				bullets.push_back(bullet);
			}
			return bullets;
		}
	
	};

	struct WeaponShotgun : public Weapon
	{
		WeaponShotgun() : Weapon() { m_type = Shotgun; }

		std::vector<Bullet*> createBullets(float aimAngle, glm::vec2 position)
		{
			std::vector<Bullet*> bullets;
			for (unsigned i = 0; i < 6; i++)
			{
				glm::vec2 vectorAngle = radToVec(aimAngle);
				Bullet* bullet = new Bullet;
				bullet->m_type = ShotgunBullet;
				bullet->m_creationDelay = 0;
				bullet->m_rotation = aimAngle; // not really needed
				bullet->m_impulse.x = vectorAngle.x * 10; bullet->m_impulse.y = vectorAngle.y * 10;
				// Shotgun does not know playerposition, has to be moved +playerposition.x &.y later
				//  Adjust creation spot according to aim!
				bullet->m_position.x = position.x + vectorAngle.x * 72; bullet->m_position.y = position.y + vectorAngle.y * 72;
				bullets.push_back(bullet);
			}
			return bullets;
		}
	};
}