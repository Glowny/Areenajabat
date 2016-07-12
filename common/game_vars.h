#pragma once

#include "forward_declare.h"
#include "types.h"

FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Ini)

namespace arena
{
	struct RespawnType final
	{
		enum Enum : uint32
		{
			Random,
			Team
		};
	};

	struct VictoryCondition final
	{
		enum Enum : uint32
		{
			MostKills,
			TeamRoundWins
		};
	};

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
	const char* const GM_BUY_ANYWHERE			= "gm_buy_anywhere";
	const char* const GM_SPAWN_PROTECTION		= "gm_spawn_protection";
	const char* const GM_VICTORY_CONDITION		= "gm_victory_condition";

	struct GameVars final
	{
		uint32				m_sv_address;
		uint32				m_sv_port;
		
		float32				m_sv_gravity;

		uint32				m_gm_max_players;
		RespawnType			m_gm_respawn_type;
		VictoryCondition	m_gm_victory_condition;

		uint32				m_gm_rounds_count;

		uint64				m_gm_round_duration;
		uint64				m_gm_spawn_protection;
		
		bool				m_gm_buy_anywhere;
		bool				m_gm_teams;
		bool				m_sv_friendly_fire;
		bool				m_sv_headshots_only;
	
		GameVars(const Ini& ini);
		~GameVars();
	};
}