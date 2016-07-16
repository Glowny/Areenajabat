#pragma once

namespace arena
{
	struct NetworkEntity
	{
		bool m_dirty				{ false };

		NetworkEntity()				= default;
		
		virtual ~NetworkEntity()	= default;
	};
}