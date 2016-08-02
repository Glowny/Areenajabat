#pragma once

#include "../../mem/pool_allocator.h"
#include "component_manager.h"
#include "../transform.h"
#include "../../singleton.h"

namespace arena
{
	class TransformManager final : public ComponentManager<Transform>
	{
	public:
		MAKE_SINGLETON(TransformManager)
	};
}