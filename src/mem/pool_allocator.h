#pragma once

#include "..\arena_types.h"
#include "pool_page.h"

#include <vector>

template<typename T>
using PageList = std::vector<PoolPage<T>>;

namespace arena
{
	template<typename T>
	class PoolAllocator final 
	{
	public:
		PoolAllocator(const uint64 initialPagesCount, const uint64 pageSize) : pageSize(pageSize) 
		{
			for (auto i = 0; i < initialPagesCount; i++) m_pages.push_back(PoolPage(pageSize));
		}

		T* allocate()
		{
			// Try to allocate from existing pages.
			for (auto& page : m_pages) 
			{
				const auto element = page.allocate();

				if (element != nullptr) return element;
			}

			// Add new page.
			m_pages.push_back(PoolPage(m_pageSize));

			const auto page = &m_pages.back();

			return page->allocate();
		}
		bool deallocate(const T* const element)
		{
			const auto address = static_cast<UintPtr>(element);

			for (auto& page : m_pages)
			{
				if (page.isInAddressSpace(address))
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
		PageList<T>		m_pages;

		const uint64	m_pageSize;
	};
}