#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"
// Weapons include for bullet type.
#include "common/arena/weapons.h"
#include "physics_component.h"
#include "component.h"

namespace arena
{
	class Projectile final : public PhysicsComponent
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(Projectile)
	
	public:
		Bullet bullet;
		~Projectile() = default;
		Projectile() = default;
	protected:
	};
}
