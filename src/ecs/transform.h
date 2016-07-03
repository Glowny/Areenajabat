#pragma once

#include "..\3rdparty\glm\vec2.hpp"
#include "..\arena_types.h"

#include "component.h"

namespace arena
{
	class Transform final : public Component
	{
	DEFINE_RTTI_SUB_TYPE(Transform)
	public:
		glm::vec2 m_position { 0.0f, 0.0f };
		glm::vec2 m_origin	 { 0.0f, 0.0f };
		glm::vec2 m_scale	 { 1.0f, 1.0f };

		float32 rotation	 { 0.0f };

		~Transform() = default;
	};

	REGISTER_RTTI_SUB_TYPE(Transform)
}
