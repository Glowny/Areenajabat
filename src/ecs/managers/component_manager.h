#pragma once

#include "..\..\forward_declare.h"

#include <cassert>
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

		virtual ~ComponentManager() = default;
	protected:
		ComponentManager() = default;

		// To handle component specific update logic.
		virtual void onUpdate(const GameTime& gameTime)
		{
		}

		virtual void onRegister(Component* const component)
		{
		}
		virtual void onUnregister(Component* const component)
		{
		}

		std::vector<T*> components()
		{
			return m_components;
		}
	private:
		std::vector<T*> m_components;
	};
}