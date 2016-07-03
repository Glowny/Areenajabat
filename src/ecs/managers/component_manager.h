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
	// Base class for component managers.
	// Not all components need managers,
	// but some do. Managers contain the
	// logic for updating and presenting the
	// components. All managers are singletons.
	template<typename T>
	class ComponentManager
	{
	public:
		T* create(Entity* const owner)
		{
			T* component = m_allocator.allocate();

			DYNAMIC_NEW(component, T, (owner));

			return component;
		}
		bool release(T* const component)
		{
			if (component == nullptr) return;

			if (!component->destroyed())
			{
				component->destroy();

				unregisterComponent(component);
			}

			m_allocator.deallocate(component);
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

				if (component->destroyed()) m_components.erase(it);
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
		ComponentManager() = default;

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
		PoolAllocator<T> m_allocator;
		
		std::vector<T*> m_components;
	};
}