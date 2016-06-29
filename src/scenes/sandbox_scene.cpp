#include "sandbox_scene.h"

#ifdef _DEBUG
#	include <iostream>
#endif

namespace arena
{
	SandboxSecene::SandboxSecene() : Scene("sandbox")
	{
	}

	void SandboxSecene::onUpdate(const GameTime& time)
	{
	}
	void SandboxSecene::onInitialize()
	{
#ifdef _DEBUG
		std::cout << "INIT" << std::endl;
#endif
	}
	void SandboxSecene::onDestroy()
	{
#ifdef _DEBUG
		std::cout << "DESTROY" << std::endl;
#endif
	}
}