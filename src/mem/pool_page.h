#pragma once

#include "..\arena_types.h"

#include <assert.h>
#include <stack>
#include <new>

template<typename T>
using HandleStack = std::stack<T>;

namespace arena
{
	template<typename T>
	class PoolPage final
	{
	public:
		PoolPage(const uint32 pageSize) : m_pageSize(pageSize),
										  m_pagePointer(0)
		{
			m_memory		= new char[pageSize * sizeof(T)];

			m_lowAddress	= reinterpret_cast<UintPtr>(&m_memory[0]);
			m_highAddress	= reinterpret_cast<UintPtr>(&m_memory[pageSize * sizeof(T)]);
		}

		// Returns an uninitialized handle to element
		// of type T.
		T* allocate() 
		{
			if (canAllocateFromReleased()) 
			{
				// Alloc from released.
				T* handle = reinterpret_cast<T*>(m_releasedHandles.top());
				
				m_releasedHandles.pop();

				return handle;
			} 
			else if (canAllocateFromPool()) 
			{
				// Alloc from pool.
				T* handle = reinterpret_cast<T*>(&m_memory[m_pagePointer]);
				
				m_pagePointer += sizeof(T);

				return handle;
			}


			// Could not allocate.
			return nullptr;
		}
		// Deallocates the given element.
		// Calls destructor.
		void deallocate(T* const element)
		{
			Char* handle = reinterpret_cast<Char*>(element);

			m_releasedHandles.push(handle);
		}

		bool isInAddressSpace(const T* const element) const 
		{
			const auto address = reinterpret_cast<UintPtr>(element);

			return address >= m_lowAddress && address <= m_highAddress;
		}

		~PoolPage() 
		{
			delete m_memory;
		}

		PoolPage(PoolPage& other) = delete;
		PoolPage(PoolPage&& other) = delete;

		PoolPage& operator =(PoolPage& other) = delete;
		PoolPage& operator =(PoolPage&& other) = delete;
	private:
		bool canAllocateFromReleased() const
		{
			return !m_releasedHandles.empty();
		}
		bool canAllocateFromPool() const 
		{
			return m_pagePointer < m_pageSize;
		}

		HandleStack<Char*>	m_releasedHandles;
		Char*				m_memory;			// Page memory.

		UintPtr	m_lowAddress;		// Low address of the page.
		UintPtr	m_highAddress;		// High address of the page.
		uint64	m_pagePointer;		// Pointer poinint to the current element of the page (bytes).
		uint32	m_pageSize;			// Page size in elements (elements).
	};
}