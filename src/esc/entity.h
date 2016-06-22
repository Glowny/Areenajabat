#pragma once

#include "..\arena_types.h"

#include <vector>

namespace arena
{
	class Entity final {
	public:
		Entity(const uint64 id);

		uint64 id() const;

		~Entity();
	private:
		std::vector<Component*> m_components;

		const uint64 m_id;
	};
}