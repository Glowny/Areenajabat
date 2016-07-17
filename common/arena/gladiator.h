#pragma once

#include "weapons.h"

#include <common/network_entity.h>

namespace arena
{
	struct Gladiator : public NetworkEntity
	{
		Gladiator() : NetworkEntity(NetworkEntityType::Gladiator) { m_alive = true; m_hitpoints = 100; }

		unsigned m_physicsId;
		
		glm::vec2 m_position;
		glm::vec2 m_velocity;
		
		float m_rotation;
		
		Weapon* m_weapon;
		
		int m_hitpoints;
		bool m_alive;
	};
}