#pragma once

#include "..\arena_types.h"

namespace arena
{
	class Component {
	public:
		Component(const uint64 id);

		uint64 id() const;

		virtual ~Component() = default;
	private:
		const uint64 m_id;
	};
}