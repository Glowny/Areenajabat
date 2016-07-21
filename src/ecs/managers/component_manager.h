#pragma once

#include "..\..\mem\pool_allocator.h"
#include "..\..\mem\memory.h"
#include "..\..\forward_declare.h"

#include <cassert>
#include <iterator>
#include <vector>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, GameTime)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Component)

namespace arena
{
	const uint32 AllocatorInitialPages	= 1;
	const uint32 AllocatorPageSize		= 1024;
 	
	// Base class for component managers.
	// Not all components need managers,
	// but some do. Managers contain the
	// logic for updating and presenting the
	// components. All managers are singletons.
	template<typename T>
	class ComponentManager
	{
	public:
		T* create()
		{
			T* component = m_allocator.allocate();

			DYNAMIC_NEW_DEFAULT(component, T);

			return component;
		}

		void registerComponent(T* const component)
		{
			assert(component != nullptr);
			
			m_components.push_back(component);

			onRegister(component);
		}
		void unregisterComponent(T* const component)
		{
			assert(component != nullptr);

			m_components.erase(std::find(m_components.begin(), m_components.end(), component), m_components.end());

			onUnregister(component);
		}

		void update(const GameTime& gameTime)
		{
			for (auto it = m_components.begin(); it != m_components.end(); it++)
			{
				T* component = *it;

				if (component->destroyed())
				{
					m_components.erase(it);

					m_allocator.deallocate(component);
					
					component->m_owner = nullptr;
					
					DYNAMIC_DTOR(component, T);
				}
			}

			onUpdate(gameTime);
		}

		decltype(auto) begin()
		{
			return m_components.begin();
		}
		decltype(auto) end()
		{
			return m_components.end();
		}
		bool empty() const
		{
			return m_components.empty();
		}

		virtual ~ComponentManager() = default;
	protected:
		ComponentManager() : m_allocator(AllocatorInitialPages, AllocatorPageSize)
		{
		}

		// To handle component specific update logic.
		virtual void onUpdate(const GameTime&)
		{
		}

		virtual void onRegister(Component* const)
		{
		}
		virtual void onUnregister(Component* const)
		{
		}
	private:
		mem::PoolAllocator<T> m_allocator;
		
		std::vector<T*> m_components;
	};
}