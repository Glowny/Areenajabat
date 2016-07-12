#pragma once

#include <cassert>
#include <functional>
#include <vector>

namespace arena
{
	template<typename T>
	class EventHandler final
	{
	public:
		EventHandler() = default;

		void add(std::function<T> callback)
		{
#if _DEBUG
			assert(std::find(m_subs.begin(), m_subs.end(), callback) == m_subs.end());
#endif
			m_subs.push_back(callback);
		}
		void remove(std::function<T> callback)
		{
			m_subs.erase(std::find(m_subs.begin(), m_subs.end(), callback), m_subs.end());
		}

		void operator +=(std::function<T> callback) 
		{
			add(callback);
		}
		void operator -=(std::function<T> callback)
		{
			remove(callback);
		}

		void signal(std::function<std::function<T>> caller)
		{
			for (uint32 i = 0; i < m_subs; i++) caller(m_subs[i]);
		}

		~EventHandler() = default;
	private:
	};
}