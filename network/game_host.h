#pragma once

#include <common\mem\memory.h>
#include <common\forward_declare.h>
#include <common\event.h>
#include <common\game_vars.h>
#include <vector>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, ClientData)

namespace arena
{
	struct Player final
	{
		const ClientData* m_clientData { nullptr };

		bool operator ==(const Player* const lhs) const
		{
			if (lhs == nullptr) return false;

			return ADDRESSOF(lhs) == ADDRESSOF(this);
		}
		bool operator !=(const Player* const lhs) const
		{
			return !(lhs == this);
		}
	};

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

		void registerPlayer(const ClientData* const client);
		void unregisterPlayer(const ClientData* const client);

		~GameHost();
	private:
		const Player* const find(const ClientData* const client) const;

		void sessionTick(const uint64 dt);
		void gameTick(const uint64 uidt, const float64 dt);

		std::vector<Player> m_players;

		const GameVars m_vars;
		
		uint64 m_sessionElapsed;	// Time elapsed from the start of the session.
		uint64 m_gameElapsed;		// Time elapsed from the start of the game.

		bool m_sessionRunning;	// Session aka the host.
		bool m_gameRunning;		// Session can be running 
								// but the game might not be running.
								// Some modes require x amount of players
								// for the game to start.
		bool m_disposed;
		bool m_endCalled;
	};
}
