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
		PoolAllocator(const uint32 initialPagesCount, const uint32 pageSize) : m_pageSize(pageSize) 
		{
			for (uint32 i = 0; i < initialPagesCount; i++) m_pages.push_back(new PoolPage<T>(pageSize));
		}

		T* allocate()
		{
			// Try to allocate from existing pages.
			for (PoolPage<T>* page : m_pages) 
			{
				T* element = page->allocate();

				if (element != nullptr) return element;
			}

			// Add new page.
			const auto page = new PoolPage<T>(m_pageSize);
			
			m_pages.push_back(page);

			return page->allocate();
		}
		bool deallocate(T* const element)
		{
			for (PoolPage<T>* page : m_pages)
			{
				if (page->isInAddressSpace(element))
				{
					page->deallocate(element);

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

		~PoolAllocator() 
		{
			for (PoolPage<T>* page : m_pages) delete page;
		}
	private:
		std::vector<arena::PoolPage<T>*>		m_pages;

		const uint32							m_pageSize;
	};
}