#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"

#include "component.h"
#include "../common/arena/gladiator.h"

namespace arena
{
	class CharacterComponent : public Component
	{
		SET_FRIENDS

			DEFINE_RTTI_SUB_TYPE(CharacterComponent)

	public:
		Gladiator m_gladiator;
		~CharacterComponent() = default;
		CharacterComponent() = default;
	protected:
	};
}
