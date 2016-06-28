#pragma once

#include "..\arena_types.h"

namespace arena
{
	template<typename T>
	class Scene
	{
	public:
		Scene(T* impl);

		void update(float32 delta);
	private:
		T* impl;
	};
}