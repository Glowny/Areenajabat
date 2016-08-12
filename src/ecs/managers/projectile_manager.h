#pragma once

#include "../../mem/pool_allocator.h"
#include "component_manager.h"
#include "../projectile.h"
#include "../../singleton.h"

namespace arena
{
	class ProjectileManager final : public ComponentManager<Projectile>
	{
	public:
		MAKE_SINGLETON(ProjectileManager)
	protected:
		virtual void onUpdate(const GameTime& gameTime) final override;
	};
}