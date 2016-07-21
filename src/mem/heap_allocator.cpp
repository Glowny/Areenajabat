#pragma once

#include "heap_allocator.h"

#include <cassert>

namespace arena
{
	namespace mem
	{
		HeapAllocator::HeapAllocator(const uint32 initialPagesCount, const uint32 pageSize) : m_pageSize(pageSize)
		{
			for (uint32 i = 0; i < initialPagesCount; i++) m_pages.push_back(new HeapPage(m_pageSize));
		}

		Char* HeapAllocator::allocate(const uint32 bytes)
		{
			assert(bytes < m_pageSize);

			for (HeapPage* page : m_pages)
			{
				Char* handle = page->allocate(bytes);

				if (handle != nullptr) return handle;
			}

			// Add new page.
			const auto page = new HeapPage(m_pageSize);

			m_pages.push_back(page);

			return page->allocate(bytes);
		}
		bool HeapAllocator::deallocate(Char* handle, const uint32 bytes)
		{
			assert(handle != nullptr);
			assert(bytes < m_pageSize);

			for (HeapPage* page : m_pages)
			{
				if (page->deallocate(handle, bytes)) return true;
			}

			return false;
		}

		HeapAllocator::~HeapAllocator()
		{
			for (HeapPage* page : m_pages) delete page;
		}
	}
}