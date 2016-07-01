#include "sprite_manager.h"

#include <algorithm>

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
		std::sort(begin(), end(), Comparer());
	}

	void SpriteManager::onUpdate(const GameTime& gameTime)
	{
		// TODO: draw.
	}

	void SpriteManager::onRegister(Component* const component) 
	{
		invalidate();
	}
}