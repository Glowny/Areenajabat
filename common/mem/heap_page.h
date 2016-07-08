#pragma once

#include "..\types.h"



namespace arena
{
	class HeapPage final
	{
	public:
		HeapPage(const uint32 size);

		inline bool isInAddressSpace();

		~HeapPage();
	private:
		UintPtr			m_lowAddress;	// Low and high address of the
		UintPtr			m_highAddress;	// m_memory.

		const uint32	m_size;			// Size of the page in bytes.
		uint32			m_hp;			// Heap pointer.

		Char*			m_memory;		// Storage.
	};
}
