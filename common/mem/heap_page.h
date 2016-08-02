#pragma once

#include "../forward_declare.h"
#include "../types.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, HeapBlock)

#include <vector>

namespace arena
{
	class HeapPage final
	{
	public:
		inline HeapPage(const uint32 size);

		inline HeapBlock* const allocate(const uint32 bytes);
		inline bool deallocate(const HeapBlock* const block);

		inline uint32 bytes() const;
		inline uint32 freeBytes() const;
		
		inline uint32 totalBlocks() const;
		inline uint32 occupiedBlocks() const;
		inline uint32 releasedBlocks() const;

		inline ~HeapPage();
	
		HeapPage(HeapPage& other) = delete;
		HeapPage(HeapPage&& other) = delete;

		HeapPage& operator =(HeapPage& other) = delete;
		HeapPage& operator =(HeapPage&& other) = delete;
	private:
		std::vector<HeapBlock> m_released;		// Released blocks that can be used for allocations.
		std::vector<HeapBlock> m_blocks;		// Blocks that are being used.
		
		UintPtr				   m_lowAddress;	// Low and high address of the
		UintPtr				   m_highAddress;	// m_memory.

		uint32				   m_bytes;			// How many bytes are being used.
		const uint32		   m_size;			// Size of the page in bytes.
		uint32				   m_hp;			// Heap pointer.

		Char*				   m_memory;		// Storage.
	};
}

#include "heap_page.inl"
