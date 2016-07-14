#pragma once
#include <glm\vec2.hpp>
namespace arena
{
	struct PlayerController
	{
		glm::ivec2 m_movementDirection;
		float m_jumpDirection;
		bool shootFlag;
		float aimAngle;
	};
}