#pragma once

#include "../types.h"

namespace arena
{
	class HeapBlock final
	{
	public:
		uint32				m_size;
		UintPtr				m_address;
		Char*			    m_handle;
		
		inline HeapBlock(const uint32 size, Char* const handle);
		inline HeapBlock(const HeapBlock& other);

		template<typename T>
		inline T& handle() const;

		inline ~HeapBlock();

		inline void operator =(const HeapBlock& other);
		inline bool operator ==(const HeapBlock& other) const;
		inline bool operator !=(const HeapBlock& other) const;
	};
}

#include "heap_block.inl"