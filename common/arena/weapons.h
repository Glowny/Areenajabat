#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <../common/network_entity.h>

#define GLADIUSIMPULSE 10.0f
#define SHOTGUNIMPULSE 10.0f
#define GRENADEIMPULSE 50.0f
namespace arena
{
	enum WeaponType :uint8_t
	{
		Gladius,
		Shotgun,
		Gernade
	};

	enum BulletType :uint8_t
	{
		GladiusBullet,
		ShotgunBullet,
		GrenadeBullet
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
		~Bullet()
		{
			delete m_position;
		}
		BulletType m_type;
		uint32_t m_ownerId;
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
			m_hitType = 0;
			m_damageAmount = 0;
			m_hitDirection = 0;
			m_hitPosition = glm::vec2(0, 0);
		}
		uint8_t m_hitType;
		uint8_t m_hitId;
		int32 m_damageAmount;
		uint8_t m_hitDirection;
		uint8_t m_targetPlayerId;
		glm::vec2 m_hitPosition;

	};

	struct Weapon : public NetworkEntity
	{
	public:
		WeaponType m_type;
		Weapon() : NetworkEntity(NetworkEntityType::Weapon)
		{
			m_coolDownTimer = 0;
			m_reloadBulletAmount = 0;
			m_currentBulletAmount = m_reloadBulletAmount;
			m_reloadTimer = 0;
			m_reloading = false;
		}
		
		std::vector<Bullet*> shoot(float aimAngle, glm::vec2 position)
		{
			std::vector<Bullet*> bullets = createBullets(aimAngle, position);
			m_currentBulletAmount -= (unsigned)bullets.size();
			return bullets;
		}

		Bullet* pitch(float aimAngle, glm::vec2 position)
		{
			Bullet* bullet = createBullet(aimAngle, position);
			return bullet;
		}

		virtual std::vector<Bullet*> createBullets(float aimAngle, glm::vec2 position) 
		{ 
			aimAngle; 
			position; 
			std::vector<Bullet*> temp; 
			return temp; 
		}

		virtual Bullet* createBullet(float aimAngle, glm::vec2 position)
		{
			aimAngle;
			position;
			Bullet* temp = new Bullet;
			return temp;
		}

		inline void finishReload()
		{
			m_currentBulletAmount = m_reloadBulletAmount;
		}
		inline void startReload()
		{
			m_reloading = true;
		}


		inline bool checkIfCanShoot(float deltaTime)
		{
			if (m_reloading)
			{
				if (checkReload(deltaTime))
				{
					m_reloading = false;
					finishReload();
					return true;
				}
				else
					return false;
			}
			if (m_currentBulletAmount == 0)
			{
				//m_reloading = true;
				return false;
			}
			if (checkCoolDown(deltaTime))
				return true;
			else
				return false;
		}

		inline bool checkCoolDown(float deltaTime)
		{
			if ((m_coolDownTimer += deltaTime) > m_coolDown)
			{ 
				m_coolDownTimer = 0;
				return true;
			}
			return false;
		}

		inline bool checkReload(float deltaTime)
		{
			if ((m_reloadTimer += deltaTime) > m_reloadTime)
			{
				m_reloadTimer = 0;
				return true;
			}
			return false;
		};

	protected:
		glm::vec2 radToVec(float r)
		{
			return glm::vec2(cos(r), sin(r));
		}
		float m_coolDown;
		float m_reloadTime;
		bool m_reloading;
		unsigned m_reloadBulletAmount;

	private:
		float m_coolDownTimer;
		float m_reloadTimer;
		unsigned m_currentBulletAmount;
	};
	

	struct WeaponGladius : public Weapon
	{
		WeaponGladius() : Weapon() 
		{ 
			m_type = Gladius; 
			m_coolDown = 0.07f;
			m_reloadTime = 1.25f;
			m_reloadBulletAmount = 30;
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
				bullet->m_impulse.x = vectorAngle.x * GLADIUSIMPULSE; bullet->m_impulse.y = vectorAngle.y * GLADIUSIMPULSE;
				bullet->m_position->x = position.x + 10; bullet->m_position->y = position.y - 16;
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
			m_coolDown = 0.4f;
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
				bullet->m_impulse.x = vectorAngle.x * SHOTGUNIMPULSE; bullet->m_impulse.y = vectorAngle.y * SHOTGUNIMPULSE;
				bullet->m_position->x = position.x + vectorAngle.x * 72; bullet->m_position->y = position.y + vectorAngle.y * 72;
				bullets.push_back(bullet);
			}
			return bullets;
		}
	};

	struct WeaponGrenade : public Weapon
	{
		WeaponGrenade() : Weapon()
		{
			m_type = Gernade;
			m_coolDown = 0.4f;
		}

		Bullet* createBullet(float aimAngle, glm::vec2 position)
		{
			glm::vec2 vectorAngle = radToVec(aimAngle);
			Bullet* bullet = new Bullet;
			bullet->m_type = GrenadeBullet;
			bullet->m_creationDelay = 0;
			bullet->m_rotation = aimAngle;
			bullet->m_impulse.x = vectorAngle.x * GRENADEIMPULSE; bullet->m_impulse.y = vectorAngle.y * GRENADEIMPULSE;
			bullet->m_position->x = position.x + vectorAngle.x * 72; bullet->m_position->y = position.y + vectorAngle.y * 72;
			return bullet;
		}
	};
}
