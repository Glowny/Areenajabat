#include "packet_allocator.h"

namespace arena
{
	PacketAllocator::PacketAllocator() : m_allocator(128, 256)
	{
	}

	Packet* const PacketAllocator::allocate(const uint32 size)
	{
		HeapBlock* const block = m_allocator.allocate(size);

		return reinterpret_cast<Packet* const>(block->m_handle);
	}
	bool PacketAllocator::deallocate(Packet* const packet)
	{
		const HeapBlock block(sizeof(packet), reinterpret_cast<Char* const>(packet));

		return m_allocator.deallocate(&block);
	}
}