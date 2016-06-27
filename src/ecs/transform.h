#pragma once

#include "..\3rdparty\glm\vec2.hpp"
#include "..\arena_types.h"

#include "component.h"

namespace arena
{
	class Transform final : public Component
	{
	public:
		glm::vec2 m_position;
		glm::vec2 m_origin;
		glm::vec2 m_scale;

		float32 rotation;
	};
}
