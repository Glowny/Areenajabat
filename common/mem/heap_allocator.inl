#pragma once

#include "heap_allocator.h"
#include "heap_block.h"
#include "heap_page.h"

#include <cassert>

namespace arena
{
	#define GET_FROM_PAGES(__what__) uint32 results = 0; for (uint32 i = 0; i < m_pages.size(); i++) results += m_pages[i]->__what__(); return results

	HeapAllocator::HeapAllocator(const uint32 initialPages, const uint32 pageSize) : m_pageSize(pageSize)
	{
		for (uint32 i = 0; i < initialPages; i++) m_pages.push_back(new HeapPage(m_pageSize));
	}

	HeapBlock* const HeapAllocator::allocate(const uint32 bytes)
	{
		assert(bytes <= m_pageSize);

		for (HeapPage* const page : m_pages)
		{
			HeapBlock* const block = page->allocate(bytes);

			if (block != nullptr) return block;
		}

		return nullptr;
	}
	bool HeapAllocator::deallocate(const HeapBlock* const block)
	{
		assert(block != nullptr);

		for (HeapPage* const page : m_pages) if (page->deallocate(block)) return true;
		
		return false;
	}

	uint32 HeapAllocator::pages() const
	{
		return uint32_t(m_pages.size());
	}

	uint32 HeapAllocator::bytes() const
	{
		GET_FROM_PAGES(bytes);
	}
	uint32 HeapAllocator::freeBytes() const
	{
		GET_FROM_PAGES(freeBytes);
	}

	uint32 HeapAllocator::totalBlocks() const
	{
		GET_FROM_PAGES(totalBlocks);
	}
	uint32 HeapAllocator::occupiedBlocks() const
	{
		GET_FROM_PAGES(occupiedBlocks);
	}
	uint32 HeapAllocator::releasedBlocks() const
	{
		GET_FROM_PAGES(releasedBlocks);
	}

	HeapAllocator::~HeapAllocator()
	{
		for (uint32 i = 0; i < m_pages.size(); i++) delete m_pages[i];

		m_pages.clear();
	}
}