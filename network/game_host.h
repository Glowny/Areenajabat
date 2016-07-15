#pragma once

#include <common\mem\memory.h>
#include <common\forward_declare.h>
#include <common\event.h>
#include <common\game_vars.h>
#include <vector>


#include <common/arena/physics.h>
#include <common/packet.h>
#include <common/arena/game_map.h>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, ClientData)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, ArenaPlatform)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, PlayerController)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, Gladiator)

namespace arena
{
	struct Entity
	{
		bool m_dirty { false };
	};

	struct Player final : public Entity
	{
		uint64					m_clientSalt		{ 0 };
		PlayerController*		m_playerController	{ nullptr };
		Gladiator*				m_gladiator			{ nullptr };
		const ClientData*		m_clientData		{ nullptr };

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

	struct GameVariables final
	{
		uint64 m_gameElapsed;		// Time elapsed from the start of the game.
		uint64 m_timeoutElapsed;
		uint64 m_roundFreezeTimeElapsed;
		uint64 m_roundsCount;
		uint64 m_roundTimeElapsed;

		bool m_gameRunning;	
		bool m_roundRunning;
		bool m_timeout;

		void resetTimers()
		{
			m_gameElapsed = m_timeoutElapsed = m_roundFreezeTimeElapsed = m_roundsCount =
			m_roundTimeElapsed = 0;
		}

		void resetState()
		{
			m_gameRunning = m_roundRunning = m_timeout = false;
		}
	};

	struct SessionVariables final
	{
		uint64 m_sessionElapsed;

		bool m_sessionRunning;

		void resetTimers()
		{
			m_sessionElapsed = 0;
		}

		void resetState()
		{
			m_sessionRunning = false;
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
		EventHandler e_kill;
		EventHandler e_respawn;
		EventHandler e_roundLimit;
		EventHandler e_sessionRestart;
		EventHandler e_timeoutStart;
		EventHandler e_timeoutEnd;

		GameHost(const GameVars& vars);
		
		void startSession();
		
		void endSession();
		void endGame();
		void dispose();

		void forceShutdown();

		bool isStateValid() const;

		void tick(const float64 dt);

		void timeOutBegin();
		void timeoutEnd();

		void registerPlayer(const ClientData* const client);
		void unregisterPlayer(const ClientData* const client);

		void processInput(const uint64 salt, const float32 x, const float32 y);
		void processShooting(const uint64 salt, const bool flags, const float32 angle);


		void clearPackets();
		const std::vector<Packet*>& getResults();

		~GameHost();
	private:
		const Player* const find(const ClientData* const client) const;

		void sessionTick(const uint64 dt);
		void gameTick(const uint64 dt);
		void worldTick(const float64 dt);

		GameMap				m_map;
		Physics				m_physics;

		std::vector<Entity*>		m_entities;
		std::vector<Player>			m_players;
		std::vector<Packet*>	m_outPackets;

		const GameVars		m_vars;

		GameVariables		m_gameData;
		SessionVariables	m_sessionData;

		bool				m_disposed;
		bool				m_endCalled;
	};
}
