#pragma once

#include "rtti_define.h"

namespace arena
{
	uint64 RTTIData::s_IDGenerator = 0u;

	RTTIData::RTTIData(const String tname) : m_typename(tname),
											 m_typeID(s_IDGenerator++) {
	}
}