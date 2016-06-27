#pragma once

#include "..\arena_types.h"

namespace arena
{
	class TypeIDManager final 
	{
	};

	class Component 
	{
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