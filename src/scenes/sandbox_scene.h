#pragma once

#include "scene.h"
#include <queue>
#include "common\arena\game_map.h"
#include "common\arena\weapons.h"

struct Message;
namespace arena
{
    
	class SandboxSecene final : public Scene 
	{
	public:
		SandboxSecene();

		~SandboxSecene() = default;
	protected:
		virtual void onUpdate(const GameTime& time) final override;
		virtual void onInitialize() final override;
		virtual void onDestroy() final override;
	};
}