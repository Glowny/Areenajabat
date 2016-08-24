#include "trail_manager.h"

#include "../transform.h"
#include "../../rtti/rtti_define.h"
#include "../../graphics/spritebatch.h"
#include "../../game_time.h"
#include "../component.h"
#include "../../app.h"
#include "../entity.h"

#include <algorithm>

namespace arena
{
	const uint32 PageSize		= 4096;
	const uint32 PagesCount		= 4;


	TrailManager::TrailManager() : ComponentManager()
	{
	}


	void TrailManager::onUpdate(const GameTime&)
	{
		// No need to draw, nothing to see here.
		if (empty()) return;

		SpriteBatch* spriteBatch = App::instance().spriteBatch();
		
		auto it			= begin();
		const auto last	= end();

		//uint32 lastLayer = (*itCur)->getLayer()

		// TODO: draw to layers.

		while (it != last) 
		{
			BulletTrail* trail = *it;

			for(unsigned i = 0; i <trail->trail.size(); i++)
			{ 
				SpriteRenderer* renderer = trail->trail[i].m_renderer;
				if (renderer->isAnchored())
				{
					Transform* const transform = trail->trail[i].m_transform;

					glm::vec2 offset = renderer->getOffset();
					glm::vec2 transformPosition = transform->m_position;

					glm::vec2& position = renderer->getPosition();
					position = transformPosition + offset;
					
				}
				if (renderer->visible())
				{ 
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
				}
			}
			it++;
		}
	}

	void TrailManager::onRegister(Component* const)
	{
		
	}
}