#pragma once

#include "Blam\Engine\Game\GameOptions.hpp"

#pragma pack(push,1)
struct s_game_globals
{
	char initializing;
	char map_active;
	__int16 sbsp_index;
	char field_4;
	char field_5;
	char field_6;
	char field_7;
	s_game_options m_options;
	char game_in_progress;
	char game_is_lost;
	PAD(2);
	int ticks_to_reset_game;
	BYTE game_options_related;	//need better name
	PAD(3);
	DWORD field_119C;		//stores 7 * game_ticks_per_second at game_finish
	DWORD field_11A0;
	PAD(68);
	DWORD field_11E8;
	PAD(126);
	WORD field_126A;
	PAD(3);
	char field_0;


	static s_game_globals* get()
	{
		return *Memory::GetAddress<s_game_globals**>(0x482D3C, 0x4CB520);
	}
	static bool map_initialized()
	{
		return get() && get()->map_active && get()->sbsp_index != 0xFFFF;
	}
	static bool game_in_progress()
	{
		return get() && get()->game_in_progress;
	}
	static s_game_options* get_game_options()
	{
		return &get()->m_options;
	}
	static s_game_variant* current_game_variant()
	{
		return &get_game_options()->m_variant;
	}
};
CHECK_STRUCT_SIZE(s_game_globals, 0x1270);
#pragma pack(pop)
