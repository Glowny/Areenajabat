#pragma once

#include <common/types.h>
#include <cassert>
#include <functional>
#include <vector>

namespace arena
{
	class EventHandler final
	{
	public:
		EventHandler() = default;

		/*for (auto i = callbacks.begin(); i != callbacks.end(); i++) {
			if ((*i).template target<T>() == callback.template target<T>()) {
				callbacks.erase(i);

				return;
			}
		}*/

		void add(std::function<void()> callback)
		{
#if _DEBUG
			for (auto i = m_subs.begin(); i != m_subs.end(); i++) {
				assert(((*i).target<void()>() != callback.target<void()>()));
			}
#endif
			m_subs.push_back(callback);
		}
		void remove(std::function<void()> callback)
		{
			for (auto i = m_subs.begin(); i != m_subs.end(); i++) {
				if ((*i).target<void()>() == callback.target<void()>()) {
					m_subs.erase(i);

					return;
				}
			}
		}

		void operator +=(std::function<void()> callback)
		{
			add(callback);
		}
		void operator -=(std::function<void()> callback)
		{
			remove(callback);
		}

		void signal()
		{
			for (uint32 i = 0; i < m_subs.size(); i++) m_subs[i]();
		}

		~EventHandler() = default;
	private:
		std::vector<std::function<void()>> m_subs;
	};
}