#pragma once

#include "../rtti/rtti_define.h"
#include "../3rdparty/glm/vec2.hpp"
#include "../arena_types.h"
#include "../camera.h"
#include "component.h"
#include "../input/input.h"

namespace arena
{
	class CameraComponent : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(CameraComponent)
	
	public:
		Camera m_camera;
		~CameraComponent() = default;
		CameraComponent() = default;
	protected:
	};
}
