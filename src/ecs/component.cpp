#include "managers\sprite_manager.h"

#include "component.h"

#include <cassert>

namespace arena
{
	INITIALIZE_RTTI_SUPER_TYPE(Component)

	Component::Component(Entity* owner) : m_owner(owner),
										  m_destroyed(false)
	{
		assert(owner != nullptr);
	}

	Entity* Component::owner() 
	{
		return m_owner;
	}

	void Component::destroy()
	{
		if (m_destroyed) return;

		m_destroyed = true;

		onDestroy();
	}

	void Component::onDestroy() 
	{
	}

	bool Component::destroyed() const
	{
		return m_destroyed;
	}

	Component::~Component()
	{
		m_owner = nullptr;
	}
}