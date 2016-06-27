#include "component.h"

#include <cassert>

namespace arena
{
	const uint32 InitialPages	= 4;
	const uint32 PageSize		= 1024 * 256;	// Page size of 256Kb. 

	HeapAllocator Component::s_allocator = HeapAllocator(InitialPages, PageSize);

	// TODO: fill rest of the RTTI system etc.
	//REGISTER_RTTI_TYPE(Component)

	// TODO: how.
	//Component* Component::create(Entity* owner, const RTTIData& type) 
	//{
	//	Char* handle = Component::s_allocator.allocate(type.m_size);
	//	
	//	Component* component = reinterpret_cast<Component*>(handle);

	//	return component;
	//}

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
		Component::s_allocator.deallocate(reinterpret_cast<Char*>(this), sizeof(this));
	}

	Component::~Component()
	{
		m_owner = nullptr;
	}
}