#pragma once

#include "component_manager.h"

#include "..\sprite_renderer.h"
#include "..\..\game_time.h"
#include "..\component.h"

#include "..\..\mem\pool_allocator.h"

#include "..\..\singleton.h"

namespace arena
{
	class SpriteManager final : public ComponentManager<SpriteRenderer>
	{
	public:
		MAKE_SINGLETON(SpriteManager)

		// Causes the manager to reorder all the sprites.
		void invalidate();
	protected:
		virtual void onUpdate(const GameTime& gameTime) final override;

		virtual void onRegister(Component* const component) final override;
	};
}