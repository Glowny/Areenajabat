#include "layers.h"

namespace arena
{
	namespace layers
	{
		bool isValidLayerID(const uint32 layerID) 
		{
			static const uint32 AllLayers = Background | Middle | Foreground | Effects | HUD;

			return (layerID & AllLayers) == 1;
		}
	}
}