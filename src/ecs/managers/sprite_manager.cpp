#include "sprite_manager.h"

#include <algorithm>

#include "..\..\game_time.h"
#include "..\component.h"

namespace arena
{
	const uint32 PageSize		= 4096;
	const uint32 PagesCount		= 4;

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

	SpriteRenderer* SpriteManager::create(Entity* const owner)
	{
		SpriteRenderer* const renderer = m_allocator.allocate();

		// Should always return a valid pointer.
		DYNAMIC_NEW(renderer, SpriteRenderer, (owner));

		return renderer;
	}
	bool SpriteManager::release(SpriteRenderer* const component)
	{
		return m_allocator.deallocate(component);
	}

	void SpriteManager::onRegister(Component* const) 
	{
		invalidate();
	}
}