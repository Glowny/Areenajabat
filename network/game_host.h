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
		EventHandler e_shutdown;

		GameHost(const GameVars& vars);
		
		void startSession();
		
		void endSession();
		void endGame();
		void dispose();

		void forceShutdown();

		bool isStateValid() const;

		void tick(const float64 dt);

		~GameHost();
	private:
		const GameVars m_vars;
		
		bool m_sessionRunning;	// Session aka the host.
		bool m_gameRunning;		// Session can be running 
								// but the game might not be running.
								// Some modes require x amount of players
								// for the game to start.
		bool m_disposed;
		bool m_endCalled;
	};
}
