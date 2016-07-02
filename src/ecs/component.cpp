#include "component.h"

#include <cassert>

namespace arena
{
	REGISTER_RTTI_TYPE(Component)

	Component::Component(Entity* owner) : m_owner(owner) 
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