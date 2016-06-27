#pragma once

#include "..\forward_declare.h"
#include "..\rtti\rtti_define.h"
#include "..\arena_types.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)

namespace arena
{
	class Component;

	class ComponentAllocator final 
	{
	};

	class Component final
	{
	DEFINE_RTTI_TYPE
	public:
		Component(Entity* owner);

		Entity* owner();

		virtual ~Component();
	private:
		/*
			Static members.
		*/

		/*
			Instance members.
		*/
		Entity* m_owner { nullptr };
	};
}