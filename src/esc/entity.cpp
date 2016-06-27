#include "..\mem\mem.h"

#include "entity.h"
#include "component.h"

/*
	Change these to optimize allocations.
*/

#define ENTITITES_INITIAL_PAGES		4
#define ENTITIES_PAGE_SIZE			1024

namespace arena
{
	/*
		Entity allocator.
	*/

	EntityAllocator::EntityAllocator(const uint64 initialPages, const uint64 pageSize) : m_allocator(initialPages, pageSize) {
	}

	Entity* EntityAllocator::create(const String& tags) {
		Entity* entity = m_allocator.allocate();
		
		// Ctor with tags call.
		DYNAMIC_NEW(entity, Entity, tags);
		
		return entity;
	}
	Entity* EntityAllocator::create() {
		Entity* entity = m_allocator.allocate();

		// No args ctro call.
		DYNAMIC_NEW_DEFAULT(entity, Entity);

		return entity;
	}

	void EntityAllocator::destroy(Entity* const entity) {
		// Deallocate, calls dtor.
		m_allocator.deallocate(entity);
	}

	/*
		Entity.
	*/

	// Init static allocator.
	EntityAllocator Entity::s_allocator = EntityAllocator(ENTITITES_INITIAL_PAGES, ENTITIES_PAGE_SIZE);

	Entity::Entity(const String& tags) : m_tags(tags) {
	}
	Entity::Entity() : m_tags(String()) {
	}

	Entity* Entity::create(const String& tags) {
		Entity* entity = Entity::s_allocator.create(tags);

		return entity;
	}
	Entity* Entity::create() {
		Entity* entity = Entity::s_allocator.create();

		return entity;
	}

	void Entity::destroy() {
		s_allocator.destroy(this);
	}

	void Entity::add(Component* const component) {
		m_components.push_back(component);
	}
	void Entity::remove(Component* const component) {
		m_components.erase(std::find(m_components.begin(), m_components.end(), component));
	}

	const String& Entity::getTags() const {
		return m_tags;
	}
	void Entity::setTags(const String& tags) {
		m_tags = tags;
	}

	ComponentIterator Entity::begin() {
		return m_components.begin();
	}
	ComponentIterator Entity::end() {
		return m_components.end();
	}
}