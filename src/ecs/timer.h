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
		float timePassed() // returns value between 0-1, 0 = no time passed.
		{
			return (m_currentTime / m_lifeTime);
		}

		float reverseTimePassed()  // returns value between 1-0, 1 = no time passed.
		{
			return (1 - timePassed());
		}

		float timePassed255() // returns value between 0-255, 0 = no time passed.
		{
			float time = timePassed() * 255;
			return time;
		}
		float timePassedReverse255() // returns value between 255-0, 255 = no time passed.
		{
			float time = reverseTimePassed() * 255;
			return time;
		}
		~Timer() = default;
		Timer() { m_lifeTime = 1; m_currentTime = 0; }
	protected:
	};
}
