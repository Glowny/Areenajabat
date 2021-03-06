#pragma once

#include <common/event.h>
#include <common/mem/memory.h>
#include <common/forward_declare.h>
#include <common/game_vars.h>
#include <vector>
#include <common/arena/physics.h>
#include <common/packet.h>
#include <common/arena/game_map.h>
#include <common/network_entity.h>
#include <common/arena/weapons.h>
#include <common/arena/scoreboard.h>
#include <common/arena/player.h>
#include <common/arena/game_mode_factory.h>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, ClientData)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, ArenaPlatform)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, PlayerController)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, Gladiator)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, PlayerInput)

#define TIMESTEP 0.016f


namespace arena
{
	enum class GameState : uint8 
	{
		
		// The game is stopped for some reason.
		Stopped	= 0,

		UnBegun,
		// The game is running but it is not 
		// executing a round yet.
		Running,

		// The game is currently executing a round.
		RoundRunning,

		// The game is currently on hold, freeze time
		// can be set manually, by the server or some
		// event that has happened.
		Freezetime,

		// The game is currently on timeout, this can
		// occur from not enough players being connected
		// to the game.
		Timeout
	};

	struct GameVariables final
	{
		uint64 m_gameElapsed;		// Time elapsed from the start of the game.
		uint64 m_timeoutElapsed;
		uint64 m_roundFreezeTimeElapsed;
		uint64 m_roundsCount;
		uint64 m_roundTimeElapsed;

		bool m_gameRunning;	

		GameState m_state;

        GameVariables() :
            m_gameElapsed(0),
            m_timeoutElapsed(0),
            m_roundFreezeTimeElapsed(0),
            m_roundsCount(0),
            m_roundTimeElapsed(0),
            m_gameRunning(false),
            m_state(GameState::Stopped)
        {

        }

		void resetTimers()
		{
			m_gameElapsed = m_timeoutElapsed = m_roundFreezeTimeElapsed = m_roundsCount =
			m_roundTimeElapsed = 0;
		}

		void resetState()
		{
			m_gameRunning = false;

			m_state = GameState::Stopped;
		}
	};

	struct SessionVariables final
	{
		uint64 m_sessionElapsed;

		bool m_sessionRunning;

        SessionVariables() :
            m_sessionElapsed(0),
            m_sessionRunning(false)
        {
        }
            

		void resetTimers()
		{
			m_sessionElapsed = 0;
		}

		void resetState()
		{
			m_sessionRunning = false;
		}
	};

	template<typename T>
	using Predicate = std::function<bool(T)>;

	template<typename T>
	class Container final
	{
	public:
		Container()
		{
		}

		bool contains(const T& element) const
		{
			for (const T entry : m_container) if (element == entry) return true;

			return false;
		}
		void add(T& element)
		{
			m_container.push_back(element);
		}
		bool remove(T& element)
		{
			for (auto it = m_container.begin(); it != m_container.end(); it++)
			{
				T& value = *it;
				
				if (value == element)
				{
					m_container.erase(it);

					return true;
				}
			}

			return false;
		}
		void at(const uint32 index)
		{
			return m_container[index];
		}
		uint32 size() const
		{
			return uint32_t(m_container.size());
		}

		T* find(Predicate<const T* const> pred)
		{
			for (T& element : m_container) if (pred(&element)) return &element;

			return nullptr;
		}

		T& front()
		{
			return m_container.front();
		}
		T& back()
		{
			return m_container.back();
		}

		void clear()
		{
			m_container.clear();
		}

        typename std::vector<T>::iterator begin()
        {
            return m_container.begin();
        }

        typename std::vector<T>::iterator end()
        {
            return m_container.end();
        }

        typename std::vector<T>::const_iterator begin() const
		{
			return m_container.begin();
		}
        typename std::vector<T>::const_iterator end() const
		{
			return m_container.end();
		}

		typename std::vector<T>& container()
		{
			return m_container;
		}

		~Container()
		{
		}
	private:
		std::vector<T> m_container;
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
		EventHandler e_gameModeSet;

		GameMode* m_gameMode;

		GameHost(const GameVars& vars);
		
		void startSession();
		
		void endSession();
		void endGame();
		void dispose();

		void forceShutdown();
		GameState getGameState();

		bool isStateValid() const;

		void tick(const float64 dt);

		void timeOutBegin();
		void timeoutEnd();

		void registerPlayer(const uint32 clientIndex);
		void unregisterPlayer(const uint32 clientIndex);

		void registerEntity(NetworkEntity* entity);
		void unregisterEntity(NetworkEntity* entity);
		// Remove entities set for removal.
		void removeAllEntites();
		void destroyEntities();

		void processInput(const uint64 clientIndex, const PlayerInput& input, float32 aimAngle);
		void GrenadeShoot(Gladiator* gladiator);
		void GladiatorShoot(Gladiator* gladiator);

		bool shouldProcessPlayerInput() const;

		void applyPlayerInputs(const float64 dt);
		void processBulletCollisions(const float64 dt);

		void loadMap(const char* const mapName);

		void selectGameMode(int number);

		NetworkEntity* const find(Predicate<NetworkEntity* const> pred);

		Scoreboard& getScoreBoard();
		std::vector<Player>& players();
		GameMap& map();
		Physics& physics();

		void getSynchronizationList(std::vector<const NetworkEntity*>& outSynchronizationList);

		~GameHost();
	private:	
		bool debugBullets = false;
		void sessionTick(const uint64 dt);
		void gameTick(const uint64 dt);
		void worldTick(const float64 dt);

		void addScoreBoard();
		void resetScoreBoard();
		void emptyScoreBoard();

		GameMap				m_map;
		Physics				m_physics;

		std::vector<const NetworkEntity*>		m_synchronizationList;
		Container<NetworkEntity*>				m_entities;
		Container<Player>						m_players;
		/*GameMode*								m_gameMode;*/
		
		const GameVars		m_vars;

		GameVariables		m_gameData;
		SessionVariables	m_sessionData;
		Scoreboard*			m_scoreBoard;

		

		bool				m_disposed;
		bool				m_endCalled;

		uint8_t getFreeEntityId();
		bool isIdFree[256]{ true };
		void nextUint8_t(uint8_t& current);
		uint8_t currentFreeId;

	};
}
