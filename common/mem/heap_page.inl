#pragma once

#include "heap_block.h"
#include "heap_page.h"
#include "memory.h"

namespace arena
{
	HeapPage::HeapPage(const uint32 size) : m_memory(new char[size]),
											m_lowAddress(ADDRESSOF(&m_memory[0])),
											m_highAddress(ADDRESSOF(&m_memory[size - 1])),
											m_size(size),
										    m_bytes(0),
											m_hp(0)
	{
	}

	HeapBlock* const HeapPage::allocate(const uint32 bytes)
	{
		if (m_hp < m_size && m_hp + bytes < m_size)
		{
			// Allocate from pool.
			Char* const handle = &m_memory[m_hp];
			// Move heap pointer.
			m_hp += bytes;

			// Create new block, return it.
			m_blocks.push_back(HeapBlock(bytes, handle));
			m_bytes += bytes;
			
			return &m_blocks.back();
		}
		else if (!m_released.empty())
		{
			// Allocate from blocks.
			for (auto it = m_released.begin(); it != m_released.end(); it++)
			{
				const HeapBlock& block = *it;

				if (bytes < block.m_size)
				{
					// Erase from released.
					m_released.erase(it);

					// Add as new block.
					m_blocks.push_back(block);

					m_bytes += bytes;

					// Return.
					return &m_blocks.back();
				}
			}
		}

		return nullptr;
	}
	bool HeapPage::deallocate(const HeapBlock* const block)
	{
		if (block->m_address >= m_lowAddress && block->m_address <= m_highAddress)
		{
			m_released.push_back(*block);

			m_blocks.erase(std::find(m_blocks.begin(), m_blocks.end(), *block), m_blocks.end());
			
			m_bytes -= block->m_size;

			return true;
		}

		return false;
	}

	uint32 HeapPage::bytes() const
	{
		return m_bytes;
	}
	uint32 HeapPage::freeBytes() const
	{
		return m_size - m_bytes;
	}

	uint32 HeapPage::totalBlocks() const
	{
		return uint32_t(m_released.size() + m_blocks.size());
	}
	uint32 HeapPage::occupiedBlocks() const
	{
		return uint32_t(m_blocks.size());
	}
	uint32 HeapPage::releasedBlocks() const
	{
		return uint32_t(m_released.size());
	}

	HeapPage::~HeapPage()
	{
		delete m_memory;
	}
}