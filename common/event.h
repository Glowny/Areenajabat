#pragma once

#include <common/types.h>
#include <assert.h>
#include <vector>
#include <bx/macros.h>
BX_PRAGMA_DIAGNOSTIC_PUSH()
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4265) // class has virtual functions, but destructor is not virtual	
#include <functional>

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
			m_subs.push_back(callback);
		}
		void remove(std::function<void()> callback)
		{
            // TODO THIS DOES NOT WORK
#if 0
			for (auto i = m_subs.begin(); i != m_subs.end(); i++) {
				if ((*i).target<void()>() == callback.target<void()>()) {
					m_subs.erase(i);

					return;
				}
			}
#endif
            (void)callback;
		}

		void operator +=(std::function<void()> callback)
		{
			add(callback);
		}
		void operator -=(std::function<void()> callback)
		{
			remove(callback);
		}

		void operator()()
		{
			signal();
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

BX_PRAGMA_DIAGNOSTIC_POP()
