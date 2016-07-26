#include "block_allocator.h"

#include <assert.h>
#include "../debug.h"

namespace arena
{
	FixedBlockAllocator::FixedBlockAllocator(const uint32 blockSize, const uint32 initialBlocks) : m_blockSize(blockSize),
																								   m_allocator(2, blockSize * initialBlocks)
	{
	}

	char* const FixedBlockAllocator::allocate()
	{
        HeapBlock* ptr = m_allocator.allocate(m_blockSize);
        ARENA_ASSERT(ptr != nullptr, "Allocation failed, out of space?");
		return ptr->m_handle;	
	}
	bool FixedBlockAllocator::deallocate(char* const handle)
	{
		HeapBlock block(m_blockSize, handle);

		if (m_blockSize == 24) {
			static volatile int i = 0;
			i++;
		}

		return m_allocator.deallocate(&block);
	}

	uint32 FixedBlockAllocator::pages() const
	{
		return m_allocator.pages();
	}

	uint32 FixedBlockAllocator::bytes() const
	{
		return m_allocator.bytes();
	}
	uint32 FixedBlockAllocator::freeBytes() const
	{
		return m_allocator.freeBytes();
	}

	uint32 FixedBlockAllocator::totalBlocks() const
	{
		return m_allocator.totalBlocks();
	}
	uint32 FixedBlockAllocator::occupiedBlocks() const
	{
		return m_allocator.occupiedBlocks();
	}
	uint32 FixedBlockAllocator::releasedBlocks() const
	{
		return m_allocator.releasedBlocks();
	}

	FixedBlockAllocator::~FixedBlockAllocator()
	{
	}

	BlockAllocator::BlockAllocator(const uint32 initialMaxBlockSize, const uint32 initialMaxBlocks) : m_initialMaxBlocks(initialMaxBlocks)
	{
		m_allocators.resize(initialMaxBlockSize);
	}

	char* const BlockAllocator::allocate(const uint32 size)
	{
		if (size == 0) return nullptr;

		// Resize if needed.
		if (size > maxBlockSize()) m_allocators.resize(size);

		// Get the allocator for the given size.
		const uint32 allocatorIndex		= size - 1;
		FixedBlockAllocator* allocator	= m_allocators[allocatorIndex];

		// No allocator for this block size, create one.
		if (allocator == nullptr)
		{
			allocator = new FixedBlockAllocator(size, m_initialMaxBlocks);
		
			m_allocators[allocatorIndex] = allocator;
		}

		// Allocate and return.
		return allocator->allocate();
	}
	bool BlockAllocator::deallocate(char* const handle, const uint32 size)
	{
		if (handle == nullptr)	return false;
		if (size == 0)			return false;

		const uint32 allocatorIndex			 = size - 1;
		FixedBlockAllocator* const allocator = m_allocators[allocatorIndex];
		
		assert(allocator != nullptr);

		return allocator->deallocate(handle);
	}

	uint32 BlockAllocator::pages() const
	{
		uint32 results = 0;

		for (FixedBlockAllocator* const allocator : m_allocators)
		{
			if (allocator == nullptr) continue;

			results += allocator->pages();
		}

		return results;
	}

	uint32 BlockAllocator::bytes() const
	{
		uint32 results = 0;

		for (FixedBlockAllocator* const allocator : m_allocators)
		{
			if (allocator == nullptr) continue;

			results += allocator->bytes();
		}

		return results;
	}
	uint32 BlockAllocator::freeBytes() const
	{
		uint32 results = 0;

		for (FixedBlockAllocator* const allocator : m_allocators)
		{
			if (allocator == nullptr) continue;

			results += allocator->freeBytes();
		}

		return results;
	}

	uint32 BlockAllocator::totalBlocks() const
	{
		uint32 results = 0;

		for (FixedBlockAllocator* const allocator : m_allocators)
		{
			if (allocator == nullptr) continue;

			results += allocator->totalBlocks();
		}

		return results;
	}
	uint32 BlockAllocator::occupiedBlocks() const
	{
		uint32 results = 0;

		for (FixedBlockAllocator* const allocator : m_allocators)
		{
			if (allocator == nullptr) continue;

			results += allocator->occupiedBlocks();
		}

		return results;
	}
	uint32 BlockAllocator::releasedBlocks() const
	{
		uint32 results = 0;

		for (FixedBlockAllocator* const allocator : m_allocators)
		{
			if (allocator == nullptr) continue;

			results += allocator->releasedBlocks();
		}

		return results;
	}

	uint32 BlockAllocator::allocators() const
	{
		uint32 results = 0;

		for (FixedBlockAllocator* const allocator : m_allocators)
		{
			if (allocator == nullptr) continue;

			results++;
		}

		return results;
	}

	uint32 BlockAllocator::maxBlockSize() const
	{
		return uint32_t(m_allocators.size());
	}

	BlockAllocator::~BlockAllocator()
	{
		for (FixedBlockAllocator* allocator : m_allocators) delete allocator;
	}
}