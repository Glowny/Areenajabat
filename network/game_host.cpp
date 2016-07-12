#include "game_host.h"

namespace arena
{
	GameHost::GameHost(const GameVars& vars) : m_vars(vars),
											   m_sessionRunning(false),
											   m_gameRunning(false),
											   m_disposed(false),
											   m_endCalled(false)
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
		// Game impl.
	}

	GameHost::~GameHost()
	{
	}
}