#pragma once

namespace arena
{
	inline HeapAllocator(const uint32 initialPages, const uint32 pageSize);

	inline HeapBlock* const allocate(const uint32 bytes);
	inline bool deallocate(const HeapBlock* const block);

	inline uint32 pages() const;

	inline uint32 bytes() const;
	inline uint32 freeBytes() const;

	inline uint32 totalBlocks() const;
	inline uint32 occupiedBlocks() const;
	inline uint32 releasedBlocks() const;

	inline ~HeapAllocator();
}