#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"

#include "component.h"

namespace arena
{
	class PhysicsRotation final : public Component
	{
		SET_FRIENDS

			DEFINE_RTTI_SUB_TYPE(PhysicsRotation)

	public:
		uint8_t m_physicsId = 0;
		float32 m_rotation = 0;

		~PhysicsRotation() = default;
		PhysicsRotation() = default;
	protected:
	};
}
