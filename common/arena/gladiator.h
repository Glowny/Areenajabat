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
		}

		glm::vec2	m_position;
		glm::vec2	m_velocity;

		float32		m_rotation;		
		uint32		m_physicsId;
		uint8_t		m_ownerId;
		int32		m_hitpoints;
		
		Weapon*		m_weapon;

		bool		m_alive;
	};
}