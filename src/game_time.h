#pragma once

#include "arena_types.h"

namespace arena
{
	class GameTime final
	{
	public:
		const float64 m_delta;
		const float64 m_total;

		GameTime(const float64 delta, const float64 total);

		~GameTime() = default;
	};
}