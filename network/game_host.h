#pragma once

#include <common\event.h>
#include <common\game_vars.h>

namespace arena
{
	class GameHost final
	{
	public:
		EventHandler e_gameStart;
		EventHandler e_gameEnd;
		EventHandler e_roundEnd;
		EventHandler e_roundStart;
		EventHandler e_sessionStart;
		EventHandler e_sessionEnd;
		EventHandler asdasdsad;

		GameHost(const GameVars& vars);
		
		void startSession();
		void endSession();

		void tick(const float64 dt);

		~GameHost();
	private:
		const GameVars m_vars;
	};
}
