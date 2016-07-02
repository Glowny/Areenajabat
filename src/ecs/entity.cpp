#include "..\mem\memory.h"

#include "entity.h"
#include "component.h"

namespace arena
{
	/*
		Change these to optimize allocations.
	*/

	const uint32 InitialPages	= 4;
	const uint32 PageSize		= Mem1K;

	/*
		Entity allocator.
	*/

	EntityAllocator::EntityAllocator(const uint32 initialPages, const uint32 pageSize) : m_allocator(initialPages, pageSize)
	{
	}

	Entity* EntityAllocator::create(const String& tags) 
	{
		Entity* entity = m_allocator.allocate();
		
		// Ctor with tags call.
		DYNAMIC_NEW(entity, Entity, tags);
		
		return entity;
	}
	Entity* EntityAllocator::create() 
	{
		Entity* entity = m_allocator.allocate();

		// No args ctro call.
		DYNAMIC_NEW_DEFAULT(entity, Entity);

		return entity;
	}

	void EntityAllocator::destroy(Entity* entity)
	{
		// Deallocate, calls dtor.
		m_allocator.deallocate(entity);
	}

	/*
		Entity.
	*/

	// Init static allocator.
	EntityAllocator Entity::s_allocator = EntityAllocator(InitialPages, PageSize);

	Entity::Entity(const String& tags) : m_tags(tags),
									     m_destroyed(false)
	{
	}
	Entity::Entity() : m_tags(String()),
					   m_destroyed(false)
	{
	}

	Entity* Entity::create(const String& tags) 
	{
		Entity* entity = Entity::s_allocator.create(tags);

		return entity;
	}
	Entity* Entity::create() 
	{
		Entity* entity = Entity::s_allocator.create();

		return entity;
	}

	void Entity::destroy()
	{
		if (m_destroyed) return;

		s_allocator.destroy(this);

		// TODO: cleanup components.
		m_destroyed = true;
	}
	bool Entity::destroyed() 
	{
		return m_destroyed;
	}

	void Entity::add(Component* const component) 
	{
		m_components.push_back(component);
	}
	void Entity::remove(Component* const component)
	{
		m_components.erase(std::find(m_components.begin(), m_components.end(), component));
	}

	bool Entity::contains(const RTTIData& componentType) const
	{
	}
	uint32 Entity::count(const RTTIData& componentType) const
	{
	}

	Component* const Entity::first(const RTTIData& componentType) const
	{
	}
	void Entity::ofType(const RTTIData& componentType, std::vector<Component*>& outResults) const
	{
	}

	Component* const Entity::find(Predicate<Component*> predicate)
	{
	}

	const String& Entity::getTags() const 
	{
		return m_tags;
	}
	void Entity::setTags(const String& tags)
	{
		m_tags = tags;
	}

	ComponentIterator Entity::begin()
	{
		return m_components.begin();
	}
	ComponentIterator Entity::end() 
	{
		return m_components.end();
	}

	bool Entity::operator ==(Entity* lhs)
	{
		if (lhs == nullptr) return false;

		const UintPtr rhsAddr = reinterpret_cast<UintPtr>(this);
		const UintPtr lhsAddr = reinterpret_cast<UintPtr>(lhs);

		return lhsAddr == rhsAddr;
	}
}