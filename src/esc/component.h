#pragma once

#include "..\arena_types.h"

namespace arena
{
	class Component {
	public:
		Component(Entity* const owner);

		virtual ~Component() = default;
	private:
		const uint64	m_id;
		Entity* const	m_owner;
	};
}