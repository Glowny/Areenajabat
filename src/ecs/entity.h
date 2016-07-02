#pragma once

#include "..\mem\pool_allocator.h"
#include "..\forward_declare.h"
#include "..\arena_types.h"
#include "..\functional.h"

#include <vector>
#include <iterator>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Component)

using ComponentIterator = std::vector<arena::Component*>::iterator;

namespace arena
{
	class Entity;

	class EntityAllocator final 
	{
	public:
		EntityAllocator(const uint32 initialPages, const uint32 pageSize);
		
		Entity* create(const String& tags);
		Entity* create();

		void destroy(Entity* const entity);

		~EntityAllocator() = default;
	private:
		PoolAllocator<Entity> m_allocator;
	};

	class Entity final 
	{
	public:
		/*
			Static members.
		*/
		static Entity* create(const String& tags);
		static Entity* create();

		/*
			Instance members.
		*/

		// TODO: hide these?
		Entity(const String& tags);
		Entity();

		void destroy();
		bool destroyed();

		void add(Component* const component);
		void remove(Component* const component);

		bool contains(const RTTIData& componentType) const;
		uint32 count(const RTTIData& componentType) const;

		Component* const first(const RTTIData& componentType) const;
		Component* const ofType(const RTTIData& componentType) const;

		Component* const find(Predicate<Component*> predicate);

		const String& getTags() const;
		void setTags(const String& tags);

		ComponentIterator begin();
		ComponentIterator end();

		~Entity()	= default;

		// TODO: overwrite new and delete operators?
		bool operator ==(Entity* lhs);
	private:
		/*
			Static members.
		*/
		static EntityAllocator s_allocator;

		/*
			Instance members.
		*/
		std::vector<Component*> m_components;

		String					m_tags;

		bool					m_destroyed;
	};
}