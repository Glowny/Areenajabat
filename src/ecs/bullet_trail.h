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
			//if (trail.size() == 0)
			//	return true;
			//else
			//	return false;
			return bulletDestroyed;
		}

		inline void updateHeadData(glm::vec2 position, glm::vec2 velocity)
		{
			m_currentHeadPosition = position;
			m_currentHeadVelocity = velocity;
			m_timePassedSinceLastPositionUpdate = 0;
		}


		inline void addPart(float rotation, Transform* transform, SpriteRenderer* renderer)
		{
			TrailPart part;
			part.m_alpha = 255;
			part.m_renderer = renderer;
			part.m_transform = transform;
			transform->m_position = m_currentHeadPosition;
			renderer->setRotation(rotation);
			glm::vec2& origin = renderer->getOrigin();
			origin = glm::vec2(15.0f, 2.5f);
			glm::vec2& scale = renderer->getScale();
			float velocity = sqrt((m_currentHeadVelocity.x * m_currentHeadVelocity.x + m_currentHeadVelocity.y * m_currentHeadVelocity.y))/7500.0f;
			
			scale.x = 0.4 + velocity;
			scale.y = 0.2f;
			trail.push_back(part);
		}

		//inline bool checkTimer()
		//{
		//	if (createNewPartTimer > 0.016f && !bulletDestroyed)
		//	{
		//		amount++;
		//		createNewPartTimer = 0;
		//		return true;
		//	}
		//	return false;
		//}
		inline void update(float dt)
		{
			m_timePassedSinceLastPositionUpdate += dt;
			m_currentHeadPosition += m_currentHeadVelocity * dt;
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
		//float createNewPartTimer = 0.0f;
		glm::vec2 m_currentHeadPosition;
		glm::vec2 m_currentHeadVelocity;
		float m_timePassedSinceLastPositionUpdate = 0.0f;
	};
}
