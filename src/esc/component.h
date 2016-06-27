#pragma once

#include "..\rtti\rtti_define.h"
#include "..\arena_types.h"

namespace arena
{
	class Component 
	{
	DEFINE_RTTI_TYPE
	public:
		Component(Entity* const owner);

		virtual ~Component() = default;
	private:
		/*
			Static members.
		*/

		/*
			Instance members.
		*/
		const uint64 m_typeID;
	};
}