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
				return true;
				m_currentRespawnTime = 0;
			}
			return false;
		}
		std::vector<Bullet*> createBullets()
		{
			return m_weapon->createBullets(m_aimAngle, *m_position);
		}

		glm::vec2*	m_position;
		glm::vec2*	m_velocity;

		float32		m_aimAngle;		
		uint32		m_physicsId;
		uint8_t		m_ownerId;
		int32		m_hitpoints;
		
		Weapon*		m_weapon;

		bool		m_alive;
	private:
		float64		m_respawnTime;
		float64		m_currentRespawnTime;
	};
}