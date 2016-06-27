#include "component.h"

#define INITIAL

namespace arena
{
	const uint32 InitialPages	= 4;
	const uint32 PageSize		= 1024 * 256;	// Page size of 256Kb. 

	HeapAllocator Component::s_allocator = HeapAllocator(InitialPages, PageSize);

	Component::Component(Entity* owner) : m_owner(owner) 
	{
	}

	Entity* Component::owner() 
	{
		return m_owner;
	}

	void Component::destroy()
	{
		Component::s_allocator.deallocate(reinterpret_cast<Char*>(this), sizeof(this));
	}

	Component::~Component()
	{
		m_owner = nullptr;
	}
}