#pragma once

#include "..\forward_declare.h"
#include "..\types.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, HeapBlock)

#include <vector>

namespace arena
{
	class HeapPage final
	{
	public:
		HeapPage(const uint32 size);

		inline HeapBlock& allocate(const uint32 bytes);
		inline void deallocate(const HeapBlock& block);

		inline bool isInAddressSpace(const HeapBlock& block) const;

		inline bool canAllocate() const;

		~HeapPage();
	private:
		UintPtr			m_lowAddress;	// Low and high address of the
		UintPtr			m_highAddress;	// m_memory.

		const uint32	m_size;			// Size of the page in bytes.
		uint32			m_hp;			// Heap pointer.

		Char*			m_memory;		// Storage.
	};
}
