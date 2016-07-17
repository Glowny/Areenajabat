#include "block_allocator.h"

#include <assert.h>

namespace arena
{
	FixedBlockAllocator::FixedBlockAllocator(const uint32 blockSize, const uint32 initialBlocks) : m_blockSize(blockSize),
																								   m_allocator(1, blockSize * initialBlocks)
	{
	}

	HeapBlock* const FixedBlockAllocator::allocate()
	{
		return m_allocator.allocate(m_blockSize);	
	}
	bool FixedBlockAllocator::deallocate(const HeapBlock* const block)
	{
		return m_allocator.deallocate(block);
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

	HeapBlock* const BlockAllocator::allocate(const uint32 size)
	{
		// Resize if needed.
		if (size > maxBlockSize()) m_allocators.resize(size);

		// Get the allocator for the given size.
		FixedBlockAllocator* allocator = m_allocators[size];

		// No allocator for this block size, create one.
		if (allocator == nullptr)
		{
			allocator = new FixedBlockAllocator(size, m_initialMaxBlocks);
		
			m_allocators[size] = allocator;
		}

		// Allocate and return.
		return allocator->allocate();
	}
	bool BlockAllocator::deallocate(const HeapBlock* const block)
	{
		FixedBlockAllocator* const allocator = m_allocators[block->m_size];
		
		assert(allocator != nullptr);

		return allocator->deallocate(block);
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