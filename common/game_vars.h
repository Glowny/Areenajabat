#pragma once

#include "forward_declare.h"
#include "types.h"

#include <map>

FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, minIni)

namespace arena
{	
	enum class RespawnType : uint32
	{
		Random = 0,
		Team
	};

	enum class VictoryCondition : uint32
	{
		MostKills = 0,
		TeamRoundWins
	};

	const uint32 DefaultPort			= 8088;
	const uint32 DefaultAddress			= 0;
	const uint32 DefaultRoundDuration	= 900;
	const uint32 DefaultRoundFreezeTime = 5;

	const char* const SECTION_SERVER			= "server";
	const char* const SV_ADDRESS				= "sv_address";
	const char* const SV_PORT					= "sv_port";
	const char* const SV_GRAVITY				= "sv_gravity";
	const char* const SV_HEADSHOTS_ONLY			= "sv_headshots_only";
	const char* const SV_FRIENDLY_FIRE			= "sv_friendly_fire";
	
	const char* const SECTION_GAMEMODE			= "gamemode";
	const char* const GM_MAX_PLAYERS			= "gm_max_players";
	const char* const GM_RESPAWN_TYPE			= "gm_respawn_type";
	const char* const GM_TEAMS					= "gm_teams";
	const char* const GM_ROUND_DURATION			= "gm_round_duration";
	const char* const GM_ROUNDS_COUNT			= "gm_rounds_count";
	const char* const GM_SPAWN_PROTECTION		= "gm_spawn_protection";
	const char* const GM_VICTORY_CONDITION		= "gm_victory_condition";
	const char* const GM_ROUND_FREEZE_TIME		= "gm_round_freeze_time";
	const char* const GM_PLAYERS_REQUIRED		= "gm_players_required";
	const char* const GM_MID_GAME_RESTART		= "gm_mid_game_restart";
	const char* const GM_PAUSE					= "gm_pause";
	const char* const GM_PLAYER_WAIT_TIME		= "gm_player_wait_time";
	const char* const GM_SHUTDOWN_AFTER_WAIT	= "gm_shutdown_after_wait";

	struct GameVars final
	{
		uint32				m_sv_address;
		uint32				m_sv_port;
		
		float32				m_sv_gravity;

		uint32				m_gm_max_players;
		RespawnType			m_gm_respawn_type;
		VictoryCondition	m_gm_victory_condition;

		uint32				m_gm_rounds_count;
		uint32				m_gm_players_required;

		uint64				m_gm_round_duration;
		uint64				m_gm_spawn_protection;
		uint64				m_gm_round_freeze_time;
		uint64				m_gm_player_wait_time;
		
		bool				m_gm_mid_game_restart;
		bool				m_gm_pause;
		bool				m_gm_teams;
		bool				m_sv_friendly_fire;
		bool				m_sv_headshots_only;
		bool				m_gm_shutdown_after_wait;
	
		GameVars(const char* const gamemodeName);
		GameVars();

		void read(const minIni& ini);

		~GameVars();
	};
}