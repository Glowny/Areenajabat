#pragma once

#include "../../mem/pool_allocator.h"
#include "component_manager.h"
#include "../physics_component.h"
#include "../../singleton.h"
#include <common/arena/physics.h>
namespace arena
{
	class PhysicsManager final : public ComponentManager<PhysicsComponent>
	{
	public:
		MAKE_SINGLETON(PhysicsManager)
			void setPhysics(Physics* physics) { m_physics = physics; };
	protected:
		virtual void onUpdate(const GameTime& gameTime) final override;
	private:
		Physics* m_physics;
	};
}