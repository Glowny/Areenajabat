#pragma once

#include "..\arena_types.h"
#include "pool_page.h"

#include <vector>

namespace arena
{
	template<typename T>
	class PoolAllocator final 
	{
	public:
		PoolAllocator(const uint64 initialPagesCount, const uint32 pageSize) : m_pageSize(pageSize) 
		{
			for (auto i = 0; i < initialPagesCount; i++) m_pages.push_back(PoolPage<T>(pageSize));
		}

		T* allocate()
		{
			// Try to allocate from existing pages.
			for (PoolPage<T>& page : m_pages) 
			{
				T* element = page.allocate();

				if (element != nullptr) return element;
			}

			// Add new page.
			m_pages.push_back(PoolPage<T>(m_pageSize));

			const auto page = &m_pages.back();

			return page->allocate();
		}
		bool deallocate(T* const element)
		{
			for (auto& page : m_pages)
			{
				if (page.isInAddressSpace(element))
				{
					page.deallocate(element);

					return true;
				}
			}

			return false;
		}

		uint32 pagesCount() const
		{
			return m_pages.size();
		}
		uint64 bytes() const 
		{
			return m_pages.size() * m_pageSize * sizeof(T);
		}

		~PoolAllocator() = default;
	private:
		std::vector<arena::PoolPage<T>>		m_pages;

		const uint32						m_pageSize;
	};
}