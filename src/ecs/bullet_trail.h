#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"
#include <vector>
#include "component.h"
#include "projectile.h"
#include "timer.h"
#include "transform.h"
#include "sprite_renderer.h"
#include "../utils/color.h"
#include "entity.h"
namespace arena
{
	struct TrailPart
	{
		float m_alpha;
		bool m_destroy = false;
		SpriteRenderer* m_renderer = nullptr;
		Transform* m_transform = nullptr;
	};
	class BulletTrail final : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(BulletTrail)
	
	public:
		unsigned amount = 0;
		std::vector<TrailPart> trail;
		bool bulletDestroyed = false;
		uint8_t bulletId = 255;
		// Check if the trail is done.


		inline bool getDone()
		{
			if (trail.size() == 0)
				return true;
			else
				return false;
		}

		inline void addPart(glm::vec2 position, float rotation, Transform* transform, SpriteRenderer* renderer, float velocity)
		{
			TrailPart part;
			part.m_alpha = 255;
			part.m_renderer = renderer;
			part.m_transform = transform;
			transform->m_position = position;
			renderer->setRotation(rotation);
			glm::vec2& scale = renderer->getScale();
			scale.x = -velocity;
			scale.y = 0.2f;
			//glm::vec2& origin = renderer->getOrigin();
			//origin.x = 15.0f;
			//origin.y = 2.50f;
			//glm::vec2& offset = renderer->getOffset();
			//offset.x = 15.0f;
			trail.push_back(part);
		}

		inline bool checkTimer()
		{
			if (createNewPartTimer > 0.016f && !bulletDestroyed)
			{
				amount++;
				createNewPartTimer = 0;
				return true;
			}
			return false;
		}
		inline void update(float dt)
		{
			createNewPartTimer += dt;
			 
			for (auto elem = trail.begin(); elem != trail.end(); )
			{
				if ((elem->m_alpha -= (dt*1000)) < 10.0f)
				{
					elem->m_renderer->destroy();
					elem->m_transform->destroy();
					trail.erase(elem);
				}
				else
				{
					elem->m_renderer->setColor(color::toABGR(255, 255, 255, static_cast<uint8_t>(elem->m_alpha)));
					elem++;
				}
				
			}
		}
		
		
		~BulletTrail() = default;
		BulletTrail() = default;
	protected:
		float createNewPartTimer = 0.0f;
	};
}
