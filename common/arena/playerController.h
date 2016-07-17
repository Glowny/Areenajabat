#pragma once
#include <glm\vec2.hpp>
#include "common\arena\weapons.h"
namespace arena
{
	struct PlayerController
	{
		glm::ivec2 m_movementDirection;
		float m_jumpDirection;
		bool shootFlag;
		float aimAngle;
		WeaponType weapon;
	};
}