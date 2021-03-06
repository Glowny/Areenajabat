#pragma once

#include "../forward_declare.h"
#include "../arena_types.h"
#include "../mem/memory.h"

#include "heap_page.h"

#include <vector>

namespace arena
{
	namespace mem
	{
		class HeapAllocator final
		{
		public:
			HeapAllocator(const uint32 initialPagesCount, const uint32 pageSize);

			Char* allocate(const uint32 bytes);
			bool deallocate(Char* handle, const uint32 bytes);

			~HeapAllocator();
		private:
			std::vector<HeapPage*>		m_pages;

			const uint32				m_pageSize;
		};
	}
}