#pragma once

#include "rtti_define.h"

#include <map>
#include <algorithm>

namespace arena
{
	uint32 Type::s_IDGenerator = 0;

	Type::Type(const char* const tname, const uint32 size) : m_typename(tname),
														     m_size(size),
															 m_typeID(s_IDGenerator++) {
	}

	bool Type::operator ==(const Type& lhs) const
	{
		return lhs.m_typeID == m_typeID;
	}
	bool Type::operator !=(const Type& lhs) const
	{
		return !(lhs == *this);
	}
}