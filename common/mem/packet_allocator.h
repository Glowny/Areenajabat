#pragma once

#include "../forward_declare.h"
#include "block_allocator.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, Packet)

namespace arena
{
	class PacketAllocator final
	{
	public:
		Packet* const allocate(const uint32 size);
		bool deallocate(Packet* const packet);

		static PacketAllocator& instance()
		{
			static PacketAllocator inst;

			return inst;
		}

		~PacketAllocator() = default;

		PacketAllocator(PacketAllocator const& copy) = delete;
		PacketAllocator& operator=(PacketAllocator const& copy) = delete;
	private:
		PacketAllocator();

		BlockAllocator m_allocator;
	};
}