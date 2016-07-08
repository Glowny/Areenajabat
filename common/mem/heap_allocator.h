#pragma once

#include "..\forward_declare.h"
#include "..\types.h"

#include <vector>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, HeapPage)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, HeapBlock)

namespace arena
{
	class HeapAllocator final
	{
	public:
		inline HeapAllocator(const uint32 initialPages, const uint32 pageSize);

		inline HeapBlock* const allocate(const uint32 bytes);
		inline bool deallocate(const HeapBlock* const block);

		uint32 pages() const;

		inline ~HeapAllocator();
	private:
		std::vector<HeapPage*> m_pages;

		const uint32 m_pageSize;
	};
}