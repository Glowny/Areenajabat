#pragma once

#include "..\types.h"

namespace arena
{
	class HeapBlock final
	{
	public:
		IntPtr m_address;
		uint32 m_bytes;

		HeapBlock() = default;
		~HeapBlock() = default;
	};
}