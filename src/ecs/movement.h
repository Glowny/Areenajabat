#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"

#include "component.h"

namespace arena
{
	class Movement final : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(Movement)
	
	public:
		glm::vec2 m_velocity { 0.0f, 0.0f };
		float m_rotationSpeed = 0.0f;

		~Movement() = default;
		Movement() = default;
	protected:
	};
}
