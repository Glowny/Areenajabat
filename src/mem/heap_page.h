#pragma once

#include "..\arena_types.h"

#include <vector>

namespace arena
{
	namespace mem
	{
		class HeapHandle;

		using HeapHandleList = std::vector<arena::mem::HeapHandle>;

		class HeapHandle final
		{
		public:
			Char*	m_handle;
			uint32	m_size;

			HeapHandle() = default;
			~HeapHandle() = default;
		};

		class HeapPage final
		{
		public:
			HeapPage(const uint32 pageSize);

			Char* allocate(const uint32 bytes);
			bool deallocate(Char* handle, const uint32 bytes);

			~HeapPage();

			HeapPage(HeapPage& other) = delete;
			HeapPage(HeapPage&& other) = delete;

			HeapPage& operator =(HeapPage& other) = delete;
			HeapPage& operator =(HeapPage&& other) = delete;
		private:
			std::vector<HeapHandle>	m_handles;	// TODO: slow O(n) handle lookup, fix if this becomes an issue.
			Char*					m_memory;

			UintPtr m_lowAddress;
			UintPtr m_highAddress;
			uint64	m_pagePointer;
			uint32	m_pageSize;
		};
	}
}