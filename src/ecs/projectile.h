#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"
// Weapons include for bullet type.
#include "common\arena\weapons.h"
#include "component.h"

namespace arena
{
	class Projectile final : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(Projectile)
	
	public:
		BulletType m_bulletType;
		uint8_t m_bulletId;
		
		~Projectile() = default;
		Projectile() = default;
	protected:
	};
}
