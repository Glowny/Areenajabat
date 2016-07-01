#pragma once

#include "component_manager.h"

#include "..\sprite_renderer.h"

namespace arena
{
	class SpriteManager : public ComponentManager<SpriteRenderer>
	{
	public:
		static SpriteManager& instance();

		// Causes the manager to reorder all the sprites.
		void invalidate();
	protected:
		virtual void onUpdate(const GameTime& gameTime) final override;

		virtual void onRegister(Component* const component) final override;
	};
}