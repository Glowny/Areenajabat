#pragma once

#include"memory.h"
#include "heap_block.h"

namespace arena
{
	HeapBlock::HeapBlock(const uint32 size, Char* const handle) : m_size(size),
																  m_address(ADDRESSOF(handle)),
																  m_handle(handle)
	{
	}
	HeapBlock::HeapBlock(const HeapBlock& other) : m_size(other.m_size),
												   m_address(other.m_address),
												   m_handle(other.m_handle)
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