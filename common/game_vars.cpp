#include "minini\minIni.h"
#include "game_vars.h"
#include <map>

namespace arena
{
	std::map<const char* const, uint32> RespawnType::s_RespawnTypeValues =
	{
		{ "random", RespawnType::Random },
		{ "team", RespawnType::Team }
	};

	std::map<const char* const, uint32> VictoryCondition::s_VictoryConditionValues =
	{
		{ "most_kills", VictoryCondition::MostKills },
		{ "team_round_wins", VictoryCondition::TeamRoundWins }
	};

	GameVars::GameVars(const minIni& ini)
	{
		m_sv_address			= ini.geti(SECTION_SERVER, SV_ADDRESS, DefaultAddress);
		m_sv_port				= ini.geti(SECTION_SERVER, SV_PORT, DefaultPort);
		m_sv_gravity			= ini.getf(SECTION_SERVER, SV_GRAVITY, 9.81);
		m_sv_headshots_only		= ini.getbool(SECTION_SERVER, SV_HEADSHOTS_ONLY, false);
		m_sv_friendly_fire		= ini.getbool(SECTION_SERVER, SV_FRIENDLY_FIRE, false);

		m_gm_max_players		= ini.geti(SECTION_GAMEMODE, GM_MAX_PLAYERS, 4);
		m_gm_respawn_type		= RespawnType::s_RespawnTypeValues[ini.gets(SECTION_GAMEMODE, GM_RESPAWN_TYPE, "teams")];
		m_gm_teams				= ini.getbool(SECTION_GAMEMODE, GM_TEAMS, false);
		m_gm_round_duration		= ini.geti(SECTION_GAMEMODE, GM_ROUND_DURATION, DefaultRoundDuration);
		m_gm_rounds_count		= ini.geti(SECTION_GAMEMODE, GM_ROUNDS_COUNT, 30);
		m_gm_buy_anywhere		= ini.geti(SECTION_GAMEMODE, GM_BUY_ANYWHERE, false);
		m_gm_spawn_protection	= ini.geti(SECTION_GAMEMODE, GM_SPAWN_PROTECTION, false);
		m_gm_victory_condition  = VictoryCondition::s_VictoryConditionValues[ini.gets(SECTION_GAMEMODE, GM_VICTORY_CONDITION, "most_kills")];
	}
	
	GameVars::~GameVars()
	{
	}
}