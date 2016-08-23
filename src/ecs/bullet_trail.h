#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"
#include <vector>
#include "component.h"

namespace arena
{
	struct TrailPart
	{
		glm::vec2 m_position;
		float m_rotation;
		uint8_t m_alpha;
		bool m_destroy = false;
	};
	class BulletTrail final : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(BulletTrail)
	
	public:
		std::vector<TrailPart> trail;

		void addPart(glm::vec2 position, float rotation)
		{
			TrailPart part;
			part.m_alpha = 255;
			part.m_position = position;
			part.m_rotation = rotation;
			trail.push_back(part);
		}
		void update(float dt)
		{
			for (auto elem = trail.begin(); elem != trail.end(); )
			{
				if ((elem->m_alpha -= dt) < 5)
				{
					trail.erase(elem);
				}
				else
					elem++;
				
			}
		}
		
		
		~BulletTrail() = default;
		BulletTrail() = default;
	protected:
	};
}
