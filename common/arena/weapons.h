#pragma once

#include <vector>
#include <math.h> // sin, cos
#include <glm/vec2.hpp>
#include <../common/network_entity.h>
#include <../common/arena/gladiator_shoulder_point.h>

#define GLADIUSIMPULSE 25.0f
#define SHOTGUNIMPULSE 13.0f
#define GRENADEIMPULSE 500.0f
#define EXPLOSIONIMPULSE 400.0f
#define SHARDIMPULSE 16.0f

namespace arena
{

	enum WeaponType :uint8_t
	{
		Gladius,
		Shotgun,
		Grenade
	};

	enum BulletType :uint8_t
	{
		GladiusBullet,
		ShotgunBullet,
		GrenadeBullet,
		ShardBullet
	};

	struct Bullet : public NetworkEntity
	{
		glm::vec2 radToVec(float r)
		{
			return glm::vec2(cos(r), sin(r));
		}
		Bullet()
			: NetworkEntity(NetworkEntityType::Projectile)
		{
			m_bulletType = BulletType(0);
			m_position = new glm::vec2(0, 0);
			m_impulse = glm::vec2(0, 0);
			m_rotation = 0;
			m_creationDelay = 0;
			m_shooterId = 0;
			m_hasPhysics = true;
		}
		~Bullet()
		{
			delete m_position;
		}
		BulletType m_bulletType;
		uint32_t m_ownerId;
		unsigned m_shooterId;
		glm::vec2* m_position;
		glm::vec2 m_impulse;
		float m_rotation;
		float m_creationDelay;
	};
	struct ShardProjectile : public Bullet
	{
		ShardProjectile()
			: Bullet()
		{
			m_hasPhysics = true;
		}
	};

	struct GrenadeProjectile : public Bullet
	{
		GrenadeProjectile()
			: Bullet()
		{
			m_timer = 0;
			m_explosionTime = 1.8f;
			m_endTime = 2.0f;
			isExplosion = false;
			m_hasPhysics = true;
		}
		std::vector<ShardProjectile*> createShards()
		{
			std::vector<ShardProjectile*> shards;
			float rotation = m_rotation;
			for (unsigned i = 0; i < 12; i++)
			{
				rotation += 1.0f;
				ShardProjectile* projectile = new ShardProjectile;
				projectile->m_bulletType = ShardBullet;
				projectile->m_creationDelay = 0;
				glm::vec2 vectorAngle = radToVec(rotation);
				projectile->m_rotation = rotation;
				projectile->m_impulse.x = vectorAngle.x * SHARDIMPULSE; projectile->m_impulse.y = vectorAngle.y * SHARDIMPULSE;
				*projectile->m_position = *m_position;
				projectile->m_shooterId = m_shooterId;
				shards.push_back(projectile);
			}
			return shards;
		}
		float m_timer;
		float m_explosionTime;
		bool isExplosion;
		float m_endTime;
		uint8_t m_explosionId;
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
		glm::vec2 radToVec(float r)
		{
			return glm::vec2(cos(r), sin(r));
		}
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
			glm::vec2 shoulderPoint = getShoulderPointWithAimAngle(aimAngle);
			position = glm::vec2(position.x - 20 + 10 + shoulderPoint.x, position.y - 55 + 14 - shoulderPoint.y);//position = glm::vec2(position.x - 20 + 10 + shoulderPoint.x, position.y - 52 + 14 - shoulderPoint.y);
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

		virtual inline bool checkCoolDown(float deltaTime)
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

		bool m_reloading;
	protected:

		float m_coolDown;
		float m_reloadTime;
		unsigned m_reloadBulletAmount;
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
			m_currentBulletAmount = 30;
		}

		std::vector<Bullet*> createBullets(float aimAngle, glm::vec2 position)
		{
			std::vector<Bullet*> bullets;
			for (unsigned i = 0; i < 1; i++)
			{
				glm::vec2 vectorAngle = radToVec(aimAngle);
				Bullet* bullet = new Bullet;
				bullet->m_bulletType = GladiusBullet;
				bullet->m_creationDelay = 0.1f * i;
				bullet->m_rotation = aimAngle;
				bullet->m_impulse.x = vectorAngle.x * GLADIUSIMPULSE; bullet->m_impulse.y = vectorAngle.y * GLADIUSIMPULSE;
				bullet->m_position->x = position.x; bullet->m_position->y = position.y;
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
				bullet->m_bulletType = ShotgunBullet;
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
#define PITCHTIME 0.8f
		WeaponGrenade() : Weapon()
		{
			m_type = WeaponType::Grenade;
			m_coolDown = 1.0f;
			m_coolDownTimer = 3.0f;
			pitchTimer = 0.0f;
			pitching = false;
		}

		GrenadeProjectile* createBullet(float aimAngle, glm::vec2 position)
		{
			glm::vec2 vectorAngle = radToVec(aimAngle);
			GrenadeProjectile* bullet = new GrenadeProjectile;
			bullet->m_bulletType = GrenadeBullet;
			bullet->m_creationDelay = 0;
			bullet->m_rotation = aimAngle;
			bullet->m_impulse.x = vectorAngle.x * GRENADEIMPULSE; bullet->m_impulse.y = vectorAngle.y * GRENADEIMPULSE;
			bullet->m_position->x = position.x + vectorAngle.x * 72; bullet->m_position->y = position.y + vectorAngle.y * 72;
			return bullet;
		}

		

		bool pitchReady(float dt)
		{
			if ((pitchTimer += dt) >= PITCHTIME)
			{
				pitchTimer = 0;
				pitching = false;
				return true;
			}
			return false;
		}
		inline bool checkCoolDown(float deltaTime)
		{
			// If throwing, do not start cooldowntimer.
			if (pitching)
				return false;
			
			if ((m_coolDownTimer += deltaTime) > m_coolDown)
			{
				return true;
			}
			return false;
		}

		inline void resetCoolDown()
		{
			m_coolDownTimer = 0;
		}
	public:
		bool pitching;
	private:
		float pitchTimer;
		
	};
}
