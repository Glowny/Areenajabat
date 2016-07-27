#pragma once

#include "..\rtti\rtti_define.h"
#include "..\arena_types.h"
#include "component.h"

namespace arena
{
	class Timer final : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(Timer)
	
	public:
		float m_lifeTime;
		float m_currentTime;
		bool timePassed(float time) 
		{
			if ((m_currentTime += time) > m_lifeTime)
				return true;

				return false;
		}
		~Timer() = default;
		Timer() { m_lifeTime = 0; m_currentTime = 0; }
	protected:
	};
}
