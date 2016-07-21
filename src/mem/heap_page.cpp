#pragma once

#include "heap_page.h"

namespace arena
{
	namespace mem
	{
		HeapPage::HeapPage(const uint32 pageSize) : m_pageSize(pageSize)
		{
			m_memory = new Char[pageSize];

			m_lowAddress = reinterpret_cast<UintPtr>(&m_memory[0]);
			m_highAddress = reinterpret_cast<UintPtr>(&m_memory[pageSize]);
		}

		Char* HeapPage::allocate(const uint32 bytes)
		{
			// Fast alloc from free memory if we can, handle lookups are slow.
			if (m_pagePointer + bytes < m_pageSize)
			{
				Char* handle = &m_memory[m_pagePointer];

				m_pagePointer += bytes;

				return handle;
			}
			else if (!m_handles.empty())
			{
				// Lookup from handle list.
				for (auto iter = m_handles.begin(); iter < m_handles.end(); iter++)
				{
					HeapHandle& handle = *iter;

					if (handle.m_size <= bytes)
					{
						m_handles.erase(iter);

						return handle.m_handle;
					}
				}
			}

			// Could not allocate, return null.
			return nullptr;
		}
		bool HeapPage::deallocate(Char* handle, const uint32 bytes)
		{
			const auto address = reinterpret_cast<UintPtr>(handle);

			if (address >= m_lowAddress && address <= m_highAddress)
			{
				m_handles.push_back(HeapHandle());

				HeapHandle& heapHandle = m_handles.back();
				heapHandle.m_handle = handle;
				heapHandle.m_size = bytes;

				return true;
			}

			return false;
		}

		HeapPage::~HeapPage()
		{
			delete m_memory;
		}
	}
}