#pragma once

#include "..\..\forward_declare.h"

#include <cassert>
#include <iterator>
#include <vector>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, GameTime)
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
		std::vector<T* const> m_components;
	};
}