#pragma once

#include "component_manager.h"

#include "..\sprite_renderer.h"
#include "..\..\game_time.h"
#include "..\component.h"

#include "..\..\mem\pool_allocator.h"

namespace arena
{
	class SpriteManager final : public ComponentManager<SpriteRenderer>
	{
	public:
		static SpriteManager& instance();

		// Causes the manager to reorder all the sprites.
		void invalidate();

		SpriteRenderer* create(Entity* const owner);
		bool release(SpriteRenderer* const component);

		SpriteManager(SpriteManager const& copy) = delete;
		SpriteManager& operator=(SpriteManager const& copy) = delete;
	protected:
		virtual void onUpdate(const GameTime& gameTime) final override;

		virtual void onRegister(Component* const component) final override;
	private:
		SpriteManager();

		PoolAllocator<SpriteRenderer> m_allocator;
	};
}