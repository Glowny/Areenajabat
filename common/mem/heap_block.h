#pragma once

#include "..\types.h"

namespace arena
{
	class HeapBlock final
	{
	public:
		const uint32		m_size;
		const IntPtr		m_address;
		const Char* const	m_handle;
		
		inline HeapBlock(const uint32 size, const Char* const handle);
		inline HeapBlock(const HeapBlock& other);

		template<typename T>
		inline T& handle() const;

		inline ~HeapBlock();
	};
}

#include "heap_block.inl"