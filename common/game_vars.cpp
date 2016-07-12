#include "minini\minIni.h"
#include "game_vars.h"
#include "mem\memory.h"

#include <map>

namespace arena
{
	struct Storage final
	{
		static std::map<String, VictoryCondition>	s_VictoryConditionValues;
		static std::map<String, RespawnType>		s_RespawnTypeValues;
	};

	std::map<String, RespawnType> Storage::s_RespawnTypeValues =
	{
		{ "random", RespawnType::Random },
		{ "team", RespawnType::Team }
	};

	std::map<String, VictoryCondition> Storage::s_VictoryConditionValues =
	{
		{ "most_kills", VictoryCondition::MostKills },
		{ "team_round_wins", VictoryCondition::TeamRoundWins }
	};

	GameVars::GameVars()
	{
		ZERO_MEMORY(this, sizeof(GameVars));
	}

	void GameVars::read(const minIni& ini)
	{
		m_sv_address			= ini.geti(SECTION_SERVER, SV_ADDRESS, DefaultAddress);
		m_sv_port				= ini.geti(SECTION_SERVER, SV_PORT, DefaultPort);
		m_sv_gravity			= ini.getf(SECTION_SERVER, SV_GRAVITY, 9.81f);
		m_sv_headshots_only		= ini.getbool(SECTION_SERVER, SV_HEADSHOTS_ONLY, false);
		m_sv_friendly_fire		= ini.getbool(SECTION_SERVER, SV_FRIENDLY_FIRE, false);

		m_gm_round_freeze_time	= ini.geti(SECTION_GAMEMODE, GM_ROUND_FREEZE_TIME, DefaultRoundFreezeTime);
		m_gm_max_players		= ini.geti(SECTION_GAMEMODE, GM_MAX_PLAYERS, 4);
		m_gm_respawn_type		= Storage::s_RespawnTypeValues[ini.gets(SECTION_GAMEMODE, GM_RESPAWN_TYPE, "teams")];
		m_gm_teams				= ini.getbool(SECTION_GAMEMODE, GM_TEAMS, false);
		m_gm_round_duration		= ini.geti(SECTION_GAMEMODE, GM_ROUND_DURATION, DefaultRoundDuration);
		m_gm_rounds_count		= ini.geti(SECTION_GAMEMODE, GM_ROUNDS_COUNT, 30);
		m_gm_buy_anywhere		= ini.getbool(SECTION_GAMEMODE, GM_BUY_ANYWHERE, false);
		m_gm_spawn_protection	= ini.geti(SECTION_GAMEMODE, GM_SPAWN_PROTECTION, false);
		m_gm_victory_condition	= Storage::s_VictoryConditionValues[ini.gets(SECTION_GAMEMODE, GM_VICTORY_CONDITION, "most_kills")];
	}
	
	GameVars::~GameVars()
	{
	}
}