#pragma once

#include "Blam\Maths\Maths.h"
#include "Blam\Engine\Objects\Objects.h"
#include "Blam\Engine\Objects\ObjectPlacementData.h"

#include "Blam\Engine\Game\GameEngine.h"
#include "Blam\Engine\Players\Players.h"

constexpr signed int NONE = -1;

enum GrenadeType
{
	Frag = 0,
	Plasma = 1
};

enum SoundType
{
	TeamChange = 1,
	GainedTheLead = 2,
	LostTheLead = 3,
	Slayer = 4
};

enum static_lod : DWORD
{
	disable = 0,
	super_low,
	low,
	medium,
	high,
	super_high,
	cinematic
};

enum game_life_cycle : int
{
	life_cycle_none,
	life_cycle_pre_game,
	life_cycle_start_game,
	life_cycle_in_game,
	life_cycle_post_game,
	life_cycle_joining,
	life_cycle_matchmaking
};

enum variant_flag_bitfield : BYTE
{
	_game_engine_teams_bit = 0,
	_game_engine_motion_sensor_bit,
	_game_engine_always_invisible_bit,
	_game_engine_round_switch_resets_map_bit,
	_game_engine_tie_resolution_bit,
	_game_engine_observers_bit,
	_game_engine_changing_teams_bit,
	_game_engine_friendly_fire_bit,
	_game_engine_overshields_on_map_bit,
	_game_engine_invisiblity_on_map_bit,
	_game_engine_grenades_on_map_bit,
	_game_engine_starting_grenades_bit,
	_game_engine_extra_damage_bit,
	_game_engine_damage_resistant_bit,
	_game_engine_force_even_teams_bit,
	_game_engine_round_setting_1_round
};
int __cdecl call_object_try_and_get_with_type(datum object_datum_index, int object_type);
int __cdecl call_unit_reset_equipment(datum unit_datum_index);
bool __cdecl call_add_object_to_sync(datum gamestate_object_datum);
void __cdecl call_hs_object_destroy(datum object_datum_index);
signed int __cdecl call_unit_inventory_next_weapon(unsigned short unit_datum_index);
bool __cdecl call_assign_equipment_to_unit(datum uint, int object_index, short unk);
void __cdecl call_object_placement_data_new(ObjectPlacementData*, datum, datum, int);
signed int __cdecl call_object_new(ObjectPlacementData*);
void call_give_player_weapon(int PlayerIndex, datum WeaponId, bool bReset);
int character_datum_from_index(BYTE index);
game_life_cycle get_game_life_cycle();

class H2MOD
{
public:
		void Initialize();
		void Deinitialize();
		void ApplyHooks(); 
		void ApplyUnitHooks();

		void team_player_indicator_visibility(bool toggle);
		int get_unit_index_from_player_index(int);
		datum get_unit_datum_from_player_index(int);
		BYTE* get_player_unit_from_player_index(int playerIndex);
		datum get_player_datum_index_from_controller_index(int controller_index);
		wchar_t* get_local_player_name(int local_player_index);
		real_point3d* get_player_unit_coords(int player_index);
		float get_distance(int, int);
		wchar_t* get_session_game_variant_name();
		wchar_t* get_player_name_from_player_index(int playerIndex);
		int get_player_index_from_name(wchar_t* playername);
		int get_player_index_from_unit_datum(datum unit_datum_index);
		BYTE get_unit_team_index(datum unit_datum_index);
		void set_unit_team_index(int unit_datum_index, BYTE team);
		void set_unit_biped(Player::Biped biped_type, int playerIndex);
		void set_unit_speed_patch(bool hackit);
		void set_unit_speed(float speed, int playerIndex);
		void set_local_team_index(int local_player_index, int team);
		BYTE get_local_team_index();
		void set_player_unit_grenades_count(int playerIndex, BYTE type, BYTE count, bool resetEquipment);
		void disable_sound(int sound);
		void custom_sound_play(const wchar_t* soundName, int delay);
		void custom_sound_play(const char* soundName, int delay);

		void toggle_weapon_pickup();
		void toggle_weapon_pickup(bool bEnable);
		void leave_session();
		void session_end();
		
		scnr_type GetMapType() { return mapType; }
		void SetMapType(scnr_type value) { mapType = value; }

		bool Server;
		std::map<std::string, bool> AchievementMap;
		std::deque<std::wstring> CustomSounds;
		
		std::mutex sound_mutex;

		std::set<int> hookedObjectDefs;
		bool isChatBoxCommand = false;
		bool drawTeamIndicators = true;

		void SetBase(DWORD base) { Base = base; }

		DWORD GetBase() { return Base; }

		DWORD GetAddress(DWORD client, DWORD server = 0)
		{
			return Base + (Server ? server : client);
		}

		template <typename T = void*>
		T GetAddress(DWORD client, DWORD server = 0)
		{
			return reinterpret_cast<T>(Base + (Server ? server : client));
		}

		DWORD Base;
		scnr_type mapType;
};

extern H2MOD* h2mod;

