#include "sprite_manager.h"

#include <algorithm>

#include "..\..\game_time.h"
#include "..\component.h"

namespace arena
{
	struct Comparer final
	{
		inline bool operator()(const SpriteRenderer* const lhs, const SpriteRenderer* const rhs)
		{
			return lhs->getLayer() < rhs->getLayer();
		}
	};

	SpriteManager& SpriteManager::instance()
	{
		static SpriteManager spriteManager;

		return spriteManager;
	}

	void SpriteManager::invalidate()
	{
		std::sort(components().begin(), components().end(), Comparer());
	}

	void SpriteManager::onUpdate(const GameTime&)
	{
		// TODO: draw.
	}

	void SpriteManager::onRegister(Component* const) 
	{
		invalidate();
	}
}