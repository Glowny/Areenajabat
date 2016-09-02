#pragma once

#include "weapons.h"

#include <../common/network_entity.h>

namespace arena
{
	struct Gladiator : public NetworkEntity
	{
	public:
		Gladiator() : NetworkEntity(NetworkEntityType::Gladiator),
					  m_alive(true),
					  m_hitpoints(100)
		{ 

			m_team = 255;
			m_position = new glm::vec2(0,0);
			m_velocity = new glm::vec2(0, 0);
			m_aimAngle = 0.0f;
			m_respawnTime = 5.0f;
			m_currentRespawnTime = 0.0f;
			m_jumpCoolDownTimer = 0;
			m_ignoreLightPlatformsTimer = 0;
			m_climbing = false;
			m_throwing = false;
			m_reloading = false;
			m_remove = false;
			m_hasPhysics = true;
			m_isClimbing = false;
			m_isReloading = false;
			m_isPitching = false;
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
		bool checkIfDoingAction()
		{
			if (!m_isClimbing && !m_isPitching && !m_isReloading)
				return true;
			return false;
		}
		// Set climbing to 0 for no climbing, 1 for left, 2 for right
		void setClimbing(uint8_t climbDir)
		{
			m_climbing = climbDir;
			if (climbDir == 0)
				m_isClimbing = false;
			else
				m_isClimbing = true;
		}
		void startPitching()
		{
			m_isPitching = true;
			m_throwing = true;
			m_grenadeWeapon->pitching = true;
		}

		void startReload()
		{
			m_weapon->startReload();
			m_reloading = true;
			m_isReloading = true;
		}
		void finishReload()
		{
			m_reloading = false;
			m_isReloading = false;
		}

		// returns true if reload is still happening.
		bool checkReload()
		{
			if (!m_weapon->m_reloading)
			{
				finishReload();	
				return false;
			}
			return true;
		}

		bool isClimbing()
		{
			return m_isClimbing;
		}

		std::vector<Bullet*> shoot()
		{
			return m_weapon->shoot(m_aimAngle, *m_position);
		}

		Bullet* pitch()
		{
			m_isPitching = false;
			return m_grenadeWeapon->pitch(m_aimAngle, *m_position);
		}

		uint8_t m_team;

		glm::vec2*	m_position;
		glm::vec2*	m_velocity;

		float		m_aimAngle;		
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
		bool		m_isClimbing;
		bool		m_isPitching;
		bool		m_isReloading;
		float64		m_respawnTime;
		float64		m_currentRespawnTime;
	};
}