#pragma once

namespace arena
{
	struct Entity
	{
		bool m_dirty{ false };

		Entity()			= default;
		
		virtual ~Entity()	= default;
	};
}