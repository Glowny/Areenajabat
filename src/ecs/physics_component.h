#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"

#include "component.h"

namespace arena
{
	class PhysicsComponent : public Component
	{
		SET_FRIENDS

			DEFINE_RTTI_SUB_TYPE(PhysicsComponent)

	public:
		uint8_t m_physicsId = 0;
		bool clientSide = false;
		~PhysicsComponent() = default;
		PhysicsComponent() = default;
	protected:
	};
}
