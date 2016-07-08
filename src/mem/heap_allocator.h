#pragma once

#include "..\forward_declare.h"
#include "..\arena_types.h"
#include "..\mem\memory.h"

#include "heap_page.h"

#include <vector>

namespace arena
{
	class HeapAllocator final 
	{
	public:
		inline HeapAllocator(const uint32 initialPagesCount, const uint32 pageSize);
		
		inline Char* allocate(const uint32 bytes);
		inline bool deallocate(Char* handle, const uint32 bytes);

		inline uint32 pages() const;

		inline uint32 bytes() const;
		inline uint32 freeBytes() const;

		inline uint32 totalBlocks() const;
		inline uint32 occupiedBlocks() const;
		inline uint32 releasedBlocks() const;

		inline ~HeapAllocator();
	private:
		std::vector<HeapPage*>		m_pages;

		const uint32				m_pageSize;
	};
}