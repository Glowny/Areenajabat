#include "layers.h"

namespace arena
{
	namespace layers
	{
		bool isValidLayerID(const uint32 layerID) 
		{
			return (layerID & Layers) == 1;
		}
	}
}