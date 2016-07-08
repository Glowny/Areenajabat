#pragma once

#include "common\types.h"

namespace arena
{
	struct Gamemode final
	{
		const char* const m_name;
		const char* const m_file;
	};

	namespace gamemodes
	{
		/*
			Predefined game modes data. 
			The so called "vanilla" game modes.
		*/

		const Gamemode Deathmatch = { "dm", "dm.ini" };
	}
}
