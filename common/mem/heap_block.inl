#pragma once

#include "heap_block.h"

namespace arena
{
	HeapBlock::HeapBlock(const uint32 size, const Char* const handle) : m_size(size),
																		m_address(reinterpret_cast<IntPtr>(handle)),
																		m_handle(handle)
	{
	}

	template<typename T>
	T& HeapBlock::handle() const
	{
		return *reinterpret_cast<T*>(m_handle);
	}

	HeapBlock::~HeapBlock()
	{
	}
}