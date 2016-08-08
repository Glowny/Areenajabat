#pragma once

#include "weapons.h"

#include <../common/network_entity.h>

namespace arena
{
	struct Gladiator : public NetworkEntity
	{
		Gladiator() : NetworkEntity(NetworkEntityType::Gladiator),
					  m_alive(true),
					  m_hitpoints(100)
		{ 
			m_position = new glm::vec2(0,0);
			m_velocity = new glm::vec2(0, 0);
			m_respawnTime = 5.0f;
			m_currentRespawnTime = 0.0f;
			m_jumpCoolDownTimer = 0;
			m_ignoreLightPlatformsTimer = 0;
			m_climbing = false;
			m_throwing = false;
			m_reloading = false;

		}
		~Gladiator()
		{
			delete m_position;
			delete m_velocity;
		}
		bool checkRespawn(float64 dt)
		{

			if (m_respawnTime < (m_currentRespawnTime += dt))
			{ 
				m_currentRespawnTime = 0;
				return true;
			}
			return false;
		}
		std::vector<Bullet*> shoot()
		{
			return m_weapon->shoot(m_aimAngle, *m_position);
		}

		Bullet* pitch()
		{
			return m_grenadeWeapon->pitch(m_aimAngle, *m_position);
		}

		glm::vec2*	m_position;
		glm::vec2*	m_velocity;

		float32		m_aimAngle;		
		uint8_t		m_ownerId;
		int32		m_hitpoints;
		
		Weapon*		m_weapon;
		WeaponGrenade* m_grenadeWeapon;

		bool		m_alive;
		float		m_jumpCoolDownTimer;
		float		m_ignoreLightPlatformsTimer;
		int8_t		m_climbing;
		bool		m_throwing;
		bool		m_reloading;
	private:
		float64		m_respawnTime;
		float64		m_currentRespawnTime;
	};
}