#pragma once

#include "..\arena_types.h"

namespace arena
{
	namespace layers
	{
		const uint32 Background = 1;	
		const uint32 Middle		= 2;	
		const uint32 Foreground = 4;	
		const uint32 Effects	= 8;
		const uint32 HUD		= 16;

		const uint32 Layers[]	= { Background, Middle, Foreground, Effects, HUD };

		bool isValidLayerID(const uint32 layerID);
	}
}