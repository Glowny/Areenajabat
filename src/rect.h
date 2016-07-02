#pragma once

#include "arena_types.h"

namespace arena
{
	template<typename T>
	struct Rect final
	{
		T x;
		T y;
		T w;
		T h;

		Rect() = default;

		T left() const
		{
			return x;
		}
		T right() const
		{
			return x + w;
		}
		T top() const
		{
			return y;
		}
		T bottom() const
		{
			return y + h;
		}

		~Rect() = default;
	};

	using Recti		= Rect<int32>;
	using Rectui	= Rect<uint32>;
	using Rectf		= Rect<float32>;
	using Rectd		= Rect<float64>;
}