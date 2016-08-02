#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <../common/network_entity.h>

#define GLADIUSIMPULSE 100.0f
#define SHOTGUNIMPULSE 100.0f

namespace arena
{
	enum WeaponType :uint8_t
	{
		Gladius,
		Shotgun,
	};

	enum BulletType :uint8_t
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
			m_bulletId = 0;
			m_position = new glm::vec2(0, 0);
			m_impulse = glm::vec2(0, 0);
			m_rotation = 0;
			m_creationDelay = 0;
		}
		Bullet::~Bullet()
		{
			delete m_position;
		}
		BulletType m_type;
		uint8_t m_bulletId;
		glm::vec2* m_position;
		glm::vec2 m_impulse;
		float m_rotation;
		float m_creationDelay;
	};

	struct BulletHit : public NetworkEntity
	{
		BulletHit()
			: NetworkEntity(NetworkEntityType::BulletHit)
		{
			m_damageAmount = 0;
			m_hitDirection = 0;
			m_hitPosition = glm::vec2(0, 0);
		}
		int32 m_damageAmount;
		uint8_t m_hitDirection;
		uint8_t m_targetPlayerId;
		glm::vec2 m_hitPosition;

	};

	struct Weapon : public NetworkEntity
	{
	public:
		WeaponType m_type;
		float coolDownTimer;
		float coolDown;
		Weapon() : NetworkEntity(NetworkEntityType::Weapon)
		{
			coolDownTimer = 0;
		}
		
		virtual std::vector<Bullet*> createBullets(float aimAngle, glm::vec2 position) 
		{ 
			aimAngle; 
			position; 
			std::vector<Bullet*> temp; 
			return temp; 
		}
		inline bool checkCoolDown(float deltaTime)
		{
			if ((coolDownTimer += deltaTime) > coolDown)
			{ 
				coolDownTimer = 0;
				return true;
			}
			return false;
		}
	protected:
		glm::vec2 radToVec(float r)
		{
			return glm::vec2(cos(r), sin(r));
		}
	};

	struct WeaponGladius : public Weapon
	{
		WeaponGladius() : Weapon() 
		{ 
			m_type = Gladius; 
			coolDown = 0.05f;
			coolDownTimer = 0;
		}

		std::vector<Bullet*> createBullets(float aimAngle, glm::vec2 position)
		{
			std::vector<Bullet*> bullets;
			for (unsigned i = 0; i < 1; i++)
			{
				glm::vec2 vectorAngle = radToVec(aimAngle);
				Bullet* bullet = new Bullet;
				bullet->m_type = GladiusBullet;
				bullet->m_creationDelay = 0.1f * i;
				bullet->m_rotation = aimAngle;
				bullet->m_impulse.x = vectorAngle.x * 10; bullet->m_impulse.y = vectorAngle.y * 10;
				bullet->m_position->x = position.x + 10 + vectorAngle.x * 20; bullet->m_position->y = position.y - 16 + vectorAngle.y * 20;
				bullets.push_back(bullet);
			}
			return bullets;
		}
	
	};

	struct WeaponShotgun : public Weapon
	{
		WeaponShotgun() : Weapon() 
		{
			m_type = Shotgun; 
			coolDown = 0.4f;
			coolDownTimer = 0;
		}

		std::vector<Bullet*> createBullets(float aimAngle, glm::vec2 position)
		{
			std::vector<Bullet*> bullets;
			for (unsigned i = 0; i < 6; i++)
			{
				glm::vec2 vectorAngle = radToVec(aimAngle);
				Bullet* bullet = new Bullet;
				bullet->m_type = ShotgunBullet;
				bullet->m_creationDelay = 0;
				bullet->m_rotation = aimAngle; 
				bullet->m_impulse.x = vectorAngle.x * 10; bullet->m_impulse.y = vectorAngle.y * 10;
				bullet->m_position->x = position.x + vectorAngle.x * 72; bullet->m_position->y = position.y + vectorAngle.y * 72;
				bullets.push_back(bullet);
			}
			return bullets;
		}
	};
}