#include "packet_allocator.h"
#include "..\packet.h"

namespace arena
{
	PacketAllocator::PacketAllocator() : m_allocator(128, 256)
	{
	}

	Packet* const PacketAllocator::allocate(const uint32 size)
	{
		char* const handle = m_allocator.allocate(size);

		return reinterpret_cast<Packet* const>(handle);
	}
	bool PacketAllocator::deallocate(Packet* const packet)
	{
		return m_allocator.deallocate(reinterpret_cast<Char* const>(packet), getMaxPacketSize(packet->getType()));
	}
}