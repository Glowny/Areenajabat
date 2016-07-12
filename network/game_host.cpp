#include "game_host.h"
#include <common\debug.h>

namespace arena
{
	GameHost::GameHost(const GameVars& vars) : m_vars(vars),
											   m_sessionRunning(false),
											   m_gameRunning(false),
											   m_disposed(false),
											   m_endCalled(false),
											   m_sessionElapsed(0),
											   m_gameElapsed(0)
	{
	}

	void GameHost::startSession() 
	{
		if (m_sessionRunning) return;

		e_sessionStart();

		m_sessionRunning = true;
	}
	void GameHost::endSession()
	{
		if (!m_sessionRunning) return;

		e_sessionEnd();

		m_sessionRunning = false;
	}

	void GameHost::endGame()
	{
		if (m_endCalled) return;

		// End the game.

		e_gameEnd();

		m_endCalled = true;
	}
	void GameHost::dispose()
	{
		if (m_disposed) return;

		// Dispose of resources.
		// Last call.

		m_disposed = true;
	}

	void GameHost::forceShutdown()
	{
		endSession();
		endGame();
		dispose();
	}

	bool GameHost::isStateValid() const
	{
		return !m_disposed && !m_endCalled;
	}

	void GameHost::tick(const float64 dt)
	{
		const uint64 uidt = uint64(dt);

		if (m_sessionRunning) sessionTick(uidt);
		if (m_gameRunning)	  gameTick(uidt, dt);
	}

	void GameHost::registerPlayer(const ClientData* const client)
	{
		ARENA_ASSERT(isStateValid(), "State isn't valid");
		ARENA_ASSERT(find(client) == nullptr, "client already registered as player");

		m_players.push_back(Player());

		Player* const newPlayer = &m_players.back();
		newPlayer->m_clientData = client;

		// TODO: register new player.
	}
	void GameHost::unregisterPlayer(const ClientData* const client)
	{
		ARENA_ASSERT(isStateValid(), "State isn't valid");
		ARENA_ASSERT(find(client) != nullptr, "client not registered as player");

		for (auto it = m_players.begin(); it != m_players.end(); it++)
		{
			const ClientData* const other = &*it->m_clientData;
			
			if (other == client) 
			{
				m_players.erase(it);
				
				return;
			}
		}
		// TODO: unregister new player.
	}

	const Player* const GameHost::find(const ClientData* const client) const
	{
		for (const Player& player : m_players) if (player.m_clientData == client) return &player;
		
		return nullptr;
	}

	void GameHost::sessionTick(const uint64 dt)
	{
		// Update session data.
	}
	void GameHost::gameTick(const uint64 uidt, const float64 dt)
	{
		// Update game data (the world etc).
	}

	GameHost::~GameHost()
	{
	}
}