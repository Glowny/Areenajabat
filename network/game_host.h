#pragma once

#include <common\event.h>
#include <common\game_vars.h>

namespace arena
{
	class GameHost final
	{
	public:
		EventHandler<void()> e_gameStart;
		EventHandler<void()> e_gameEnd;
		EventHandler<void()> e_roundEnd;
		EventHandler<void()> e_roundStart;

		GameHost(const GameVars& vars);
		
		void startSession();
		void endSession();

		void tick(const float64 dt);

		~GameHost();
	private:
		const GameVars m_vars;
	};
}
