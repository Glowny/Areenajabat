#include "component.h"

namespace arena
{
	Component::Component(Entity* owner) : m_owner(owner) 
	{
	}

	Entity* Component::owner() 
	{
		return m_owner;
	}

	Component::~Component()
	{
		m_owner = nullptr;
	}
}