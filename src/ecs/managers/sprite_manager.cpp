#include "sprite_manager.h"

#include "..\..\graphics\spritebatch.h"
#include "..\..\game_time.h"
#include "..\component.h"
#include "..\..\app.h"

#include <algorithm>

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

	SpriteManager::SpriteManager() : ComponentManager(),
									 m_allocator(PagesCount, PageSize)
	{
	}

	void SpriteManager::invalidate()
	{
		std::sort(begin(), end(), Comparer());
	}

	void SpriteManager::onUpdate(const GameTime&)
	{
		SpriteBatch* spriteBatch = App::instance().spriteBatch();
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