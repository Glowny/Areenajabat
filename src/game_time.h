#pragma once

#include "arena_types.h"

namespace arena
{
	class GameTime final
	{
	public:
		const float32 m_delta;
		const float32 m_total;

		GameTime(const float32 delta, const float32 total);

		~GameTime() = default;
	};
}