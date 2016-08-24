#pragma once

#include "component_manager.h"

#include "../sprite_renderer.h"
#include "../../game_time.h"
#include "../component.h"
#include "../bullet_trail.h"
#include "../../mem/pool_allocator.h"

#include "../../singleton.h"

namespace arena
{
	class TrailManager final : public ComponentManager<BulletTrail>
	{
	public:
		MAKE_SINGLETON(TrailManager)

	protected:
		virtual void onUpdate(const GameTime& gameTime) final override;

		virtual void onRegister(Component* const component) final override;
	};
}