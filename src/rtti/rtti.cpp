#pragma once

#include "rtti_define.h"

#include <map>
#include <algorithm>

namespace arena
{
	uint32 RTTIData::s_IDGenerator = 0;

	RTTIData::RTTIData(const String tname, const uint32 size) : m_typename(tname),
																m_size(size),
																m_typeID(s_IDGenerator++) {
	}
}