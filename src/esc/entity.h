#pragma once

#include "..\forward_declare.h"
#include "..\arena_types.h"

#include <vector>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Component)

namespace arena
{
	class Entity final {
	public:
		Entity(const uint64 id);

		uint64 id() const;

		~Entity() = default;
	private:
		std::vector<Component*> m_components;

		const uint64 m_id;
	};
}