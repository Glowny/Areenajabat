#pragma once

#include "..\arena_types.h"

#include <stack>

template<typename T>
using HandleStack = std::stack<T>;

namespace arena
{
	template<typename T>
	class PoolPage final {
	public:
		PoolPage(const uint64 pageSize) : pageSize(pageSize),
										  m_pagePointer(0) {
			m_elements		= new T[pageSize];

			m_lowAddress	= static_cast<UintPtr>(&m_elements[0]);
			m_highAddress	= static_cast<UintPtr>(&m_elements[pageSize]);
		}

		// Returns an uninitialized handle to element
		// of type T.
		T* allocate() {
			if (canAllocateFromReleased()) {
				// Alloc from released.
				const auto handle = m_releasedHandles.top();
				
				m_releasedHandles.pop();

				return handle;
			} else if (canAllocateFromPool()) {
				// Alloc from pool.
				return m_elements[m_pagePointer++];
			} 

			// Could not allocate.
			return nullptr;
		}
		// Deallocates the given element.
		// Calls destructor.
		void deallocate(T* const element) {
			m_releasedHandles.push(element);

			element->~T();
		}

		bool isInAddressSpace(const T* const element) const {
			const auto address = static_cast<UintPtr>(element);

			return address >= m_lowAddress && address <= m_highAddress;
		}

		~PoolPage() {
			delete m_memory;
		}
	private:
		bool canAllocateFromReleased() const {
			return !m_releasedHandles.empty();
		}
		bool canAllocateFromPool() const {
			return m_pagePointer < m_pageSize;
		}

		HandleStack<T*>	m_releasedHandles;
		T*				m_elements;			// Page memory.

		const UintPtr	m_lowAddress;		// Low address of the page.
		const UintPtr	m_highAddress;		// High address of the page.
		const uint64	m_pagePointer;		// Pointer poinint to the current element of the page.
		const uint64	m_pageSize;			// Page size in elements.
	};
}