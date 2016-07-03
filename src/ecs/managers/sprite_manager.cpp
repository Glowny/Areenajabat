#include "sprite_manager.h"

#include "..\..\graphics\spritebatch.h"
#include "..\..\game_time.h"
#include "..\component.h"
#include "..\..\app.h"
#include "..\entity.h"

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

	SpriteManager::SpriteManager() : ComponentManager()
	{
	}

	void SpriteManager::invalidate()
	{
		//std::sort(begin(), end(), Comparer());
	}

	void SpriteManager::onUpdate(const GameTime&)
	{
		// No need to draw, nothing to see here.
		if (empty()) return;

		SpriteBatch* spriteBatch = App::instance().spriteBatch();
		
		auto itCur			= begin();
		const auto itEnd	= end();

		//uint32 lastLayer = (*itCur)->getLayer();

		while (itCur != itEnd) {
			SpriteRenderer* renderer = *itCur;

			if (renderer->isAnchored())
			{
				//Entity* const owner = renderer->owner();

				//if (owner->be)

				//glm::vec2& position = renderer->getPosition();
				//glm::vec2& offset = renderer->getOffset();
			}

			spriteBatch->draw(
				renderer->getTexture(),
				renderer->getSource(),
				renderer->getColor(),
				renderer->getPosition(),
				renderer->getOrigin(),
				renderer->getScale(),
				renderer->getEffects(),
				renderer->getRotation(),
				0.0f);

			itCur++;
		}
	}

	void SpriteManager::onRegister(Component* const) 
	{
		invalidate();
	}
}