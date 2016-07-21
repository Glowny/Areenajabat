#pragma once

#include "heap_allocator.h"
#include "memory.h"
#include "../types.h"

#include <vector>
#include <stack>

namespace arena
{
	class FixedBlockAllocator final
	{
	public:
		FixedBlockAllocator(const uint32 blockSize, const uint32 initialBlocks);
		
		char* const allocate();
		bool deallocate(char* const handle);

		uint32 pages() const;

		uint32 bytes() const;
		uint32 freeBytes() const;

		uint32 totalBlocks() const;
		uint32 occupiedBlocks() const;
		uint32 releasedBlocks() const;

		~FixedBlockAllocator();

		FixedBlockAllocator(FixedBlockAllocator& other) = delete;
		FixedBlockAllocator(FixedBlockAllocator&& other) = delete;

		FixedBlockAllocator& operator =(FixedBlockAllocator& other) = delete;
		FixedBlockAllocator& operator =(FixedBlockAllocator&& other) = delete;
	private:
		const uint32		m_blockSize;

		HeapAllocator		m_allocator;
	};

	class BlockAllocator final
	{
	public:
		// initialMaxBlockSize: how big blocks can we allocate			   (initial, dynamic)
		// initialMaxBlocks: how many blocks of given size we can allocate (initial, dynamic)
		BlockAllocator(const uint32 initialMaxBlockSize, const uint32 initialMaxBlocks);
		
		char* const allocate(const uint32 size);
		bool deallocate(char* const handle, const uint32 size);

		inline uint32 pages() const;

		inline uint32 bytes() const;
		inline uint32 freeBytes() const;

		inline uint32 totalBlocks() const;
		inline uint32 occupiedBlocks() const;
		inline uint32 releasedBlocks() const;

		uint32 allocators() const;
		uint32 maxBlockSize() const;

		~BlockAllocator();
	
		BlockAllocator(BlockAllocator& other) = delete;
		BlockAllocator(BlockAllocator&& other) = delete;

		BlockAllocator& operator =(BlockAllocator& other) = delete;
		BlockAllocator& operator =(BlockAllocator&& other) = delete;
	private:
		const uint32 m_initialMaxBlocks;

		std::vector<FixedBlockAllocator*> m_allocators;
	};
}