#pragma once

#include "..\forward_declare.h"
#include "..\arena_types.h"

#include <vector>
#include <iterator>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Component)

using ComponentIterator = std::vector<arena::Component*>::iterator;

namespace arena
{
	class Entity final {
	public:
		Entity()	= default;

		void destroy();

		void add(Component* const component);
		void remove(Component* const component);

		const String& getTags() const;
		void setTags(const String& tags);

		ComponentIterator begin();
		ComponentIterator end();

		~Entity()	= default;
	private:
		std::vector<Component*> m_components;

		String					m_tags;
	};
}