#pragma once

#include "..\rtti\rtti_define.h"
#include "..\3rdparty\glm\vec2.hpp"
#include "..\arena_types.h"

#include "component.h"

namespace arena
{
	enum EntityIdentification
	{
		None,
		Smoke,
		MuzzleFlash,

	};

	class Id final : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(Id)
	
	public:
		EntityIdentification m_id;

		~Id() = default;
		Id() { m_id = None; }
	protected:
	};
}
