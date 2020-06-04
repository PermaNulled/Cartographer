#include "H2MOD.h"
#include "Blam/Engine/FileSystem/FiloInterface.h"
#include "H2MOD/Discord/DiscordInterface.h"
#include "H2MOD/Modules/Config/Config.h"
#include "H2MOD/Modules/HitFix/HitFix.h"
#include "H2MOD/Modules/Input/Mouseinput.h"
#include "H2MOD/Modules/MainMenu/Ranks.h"
#include "H2MOD/Modules/MapFix/MPMapFix.h"
#include "H2MOD/Modules/Console/ConsoleCommands.h"
#include "H2MOD/Modules/Networking/Memory/bitstream.h"
#include "H2MOD/Modules/OnScreenDebug/OnscreenDebug.h"
#include "H2MOD/Modules/ServerConsole/ServerConsole.h"
#include "H2MOD/Modules/Tweaks/Tweaks.h"
#include "H2MOD/Variants/GunGame/GunGame.h"
#include "H2MOD/Variants/H2X/H2X.h"
#include "H2MOD/Tags/MetaLoader/tag_loader.h"
#include "H2MOD\Modules\MapManager\MapManager.h"
#include "H2MOD/Modules/Splitscreen/VideoFixes.h"
#include "H2MOD/Modules/Splitscreen/inputfixes.h"
#include <xinput.h>
#include <sqrat/include/sqrat.h>
#include "H2MOD/Modules/Scripting/Squirrel.h"
#include <d3d9.h>

H2MOD* h2mod = new H2MOD();
GunGame* gunGame = new GunGame();
Halo2Final* h2f = new Halo2Final();
DeviceShop* device_shop = new DeviceShop();
Infection* infectionHandler = new Infection();
FireFight* fireFightHandler = new FireFight();
HeadHunter* headHunterHandler = new HeadHunter();
VariantPlayer* variant_player = new VariantPlayer();


extern int H2GetInstanceId();
std::unordered_map<int, int> object_to_variant;

bool b_H2X = false;
bool b_GunGame = false;
bool b_FireFight = false;
bool b_XboxTick = false;
bool b_Infection = false;
bool b_Halo2Final = false;
bool b_HeadHunter = false;

std::map<wchar_t*, bool&> GametypesMap
{
	{ L"h2x", b_H2X },
	{ L"ogh2", b_XboxTick },
	{ L"h2f", b_Halo2Final },
	{ L"gungame", b_GunGame },
	{ L"zombies", b_Infection },
	{ L"infection", b_Infection },
	{ L"wareconomy", b_FireFight },
	{ L"headhunter", b_HeadHunter },
	{ L"graverobber", b_HeadHunter }
};

int GAME_BUILD = 11122;
int EXECUTABLE_VERSION = 4;

int character_datum_from_index(BYTE index)
{
	DWORD global_scnr = *(DWORD*)(*h2mod->GetAddress<DWORD*>(0x479E74) + 0x17C);
	DWORD var2 = *h2mod->GetAddress<DWORD*>(0x482290);
	DWORD var3 = var2 + global_scnr;
	DWORD var4 = var3 + (8 * index) + 4;
	DWORD char_tag_datum = *(DWORD*)var4;

	return char_tag_datum;
}

int get_player_index_from_datum(datum unit_datum)
{
	DatumIterator<ObjectHeader> objectsIt(game_state_objects_header);
	BipedObjectDefinition *unit_object = (BipedObjectDefinition*)objectsIt.get_data_at_index(unit_datum.ToAbsoluteIndex())->object;
	return unit_object->PlayerDatum.ToAbsoluteIndex();
}



game_life_cycle get_game_life_cycle()
{
	typedef game_life_cycle(__cdecl get_lobby_state)();
	auto p_get_lobby_state = h2mod->GetAddress<get_lobby_state*>(0x1AD660);

	return p_get_lobby_state();
}

#pragma region engine calls

int __cdecl call_object_try_and_get_with_type(datum object_datum_index, int object_type)
{
	//LOG_TRACE_GAME("call_get_object( object_datum_index: %08X, object_type: %08X )", object_datum_index, object_type);

	typedef int(__cdecl get_object)(datum object_datum_index, int object_type);
	auto p_get_object = h2mod->GetAddress<get_object*>(0x1304E3, 0x11F3A6);
	return p_get_object(object_datum_index, object_type);
}

int __cdecl call_entity_datum_to_gamestate_datum(int entity_datum)
{
	typedef int(__cdecl *entity_datum_to_gamestate_datum)(int entity_datum);
	entity_datum_to_gamestate_datum pentity_datum_to_gamestate_datum = (entity_datum_to_gamestate_datum)h2mod->GetAddress(0x1F2211);

	return pentity_datum_to_gamestate_datum(entity_datum);
}

int __cdecl call_unit_reset_equipment(datum unit_datum_index)
{
	//LOG_TRACE_GAME("unit_reset_equipment(unit_datum_index: %08X)", unit_datum_index);
	typedef int(__cdecl unit_reset_equipment)(datum unit_datum_index);
	auto p_unit_reset_equipment = h2mod->GetAddress<unit_reset_equipment*>(0x1441E0, 0x133030);
	if (unit_datum_index != -1 && unit_datum_index != 0)
	{
		return p_unit_reset_equipment(unit_datum_index);
	}

	return 0;
}

void __cdecl call_hs_object_destroy(datum object_datum_index)
{
	//LOG_TRACE_GAME("hs_object_destory(object_datum_index: %08X)", object_datum_index);
	typedef void(__cdecl hs_object_destroy)(datum object_datum_index);
	auto p_hs_object_destroy = h2mod->GetAddress<hs_object_destroy*>(0xFDCFD, 0x124ED5); // update dedi

	return p_hs_object_destroy(object_datum_index);
}

signed int __cdecl call_unit_inventory_next_weapon(unsigned short unit)
{
	//LOG_TRACE_GAME("unit_inventory_next_weapon(unit: %08X)", unit);
	typedef signed int(__cdecl unit_inventory_next_weapon)(unsigned short unit);
	auto p_unit_inventory_next_weapon = h2mod->GetAddress<unit_inventory_next_weapon*>(0x139E04);

	return p_unit_inventory_next_weapon(unit);
}

bool __cdecl call_assign_equipment_to_unit(datum unit, int object_index, short unk)
{
	//LOG_TRACE_GAME("assign_equipment_to_unit(unit: %08X,object_index: %08X,unk: %08X)", unit, object_index, unk);
	typedef bool(__cdecl assign_equipment_to_unit)(datum unit, int object_index, short unk);
	auto passign_equipment_to_unit = h2mod->GetAddress<assign_equipment_to_unit*>(0x1442AA, 0x1330FA);

	return passign_equipment_to_unit(unit, object_index, unk);
}

void __cdecl call_object_placement_data_new(ObjectPlacementData* s_object_placement_data, datum object_definition_index, datum object_owner, int unk)
{
	//LOG_TRACE_GAME("object_placement_data_new(s_object_placement_data: %08X,",s_object_placement_data);
	//LOG_TRACE_GAME("object_definition_index: %08X, object_owner: %08X, unk: %08X)", object_definition_index, object_owner, unk);

	typedef void(__cdecl object_placement_data_new)(void*, datum, datum, int);
	auto pobject_placement_data_new = h2mod->GetAddress<object_placement_data_new*>(0x132163, 0x121033);

	pobject_placement_data_new(s_object_placement_data, object_definition_index, object_owner, unk);
}

signed int __cdecl call_object_new(ObjectPlacementData* pObject)
{
	//LOG_TRACE_GAME("object_new(pObject: %08X)", pObject);

	typedef int(__cdecl object_new)(void*);
	auto p_object_new = h2mod->GetAddress<object_new*>(0x136CA7, 0x125B77);

	return p_object_new(pObject);

}

bool __cdecl call_add_object_to_sync(datum gamestate_object_datum)
{
	typedef int(__cdecl add_object_to_sync)(datum gamestate_object_datum);
	auto p_add_object_to_sync = h2mod->GetAddress<add_object_to_sync*>(0x1B8D14, 0x1B2C44);

	return p_add_object_to_sync(gamestate_object_datum);
}

/* We should really make this stuff into a struct/class, and access it that way it'd be much cleaner... */
int get_actor_datum_from_unit_datum(int unit_datum)
{
	int unit_ptr = call_object_try_and_get_with_type(unit_datum, 3);
	if (unit_ptr)
	{
		return *(int*)((BYTE*)unit_ptr + 0x130);
	}

	return -1;
}

/* This looks at the actors table to get the character datum which is assigned to the specific actor. */
int get_char_datum_from_actor(int actor_datum)
{
	__int16 actor_index = actor_datum & 0xFFFF;
	DWORD actor_table_ptr = *h2mod->GetAddress<DWORD*>(0xA965DC);
	DWORD actor_table = *(DWORD*)((BYTE*)actor_table_ptr + 0x44);
	DWORD actor = (DWORD)((BYTE*)actor_table + (actor_index * 0x898));
	int character_datum = *(int*)((BYTE*)actor+0x54);

	return character_datum;
}

/*This is to get the datum of the last player who damaged the datum/unit provided */
int get_damage_owner(int damaged_unit_index)
{
	int damaged_player_ptr = call_object_try_and_get_with_type(damaged_unit_index, 3);
	if (damaged_player_ptr)
	{
		return *(int*)((BYTE*)damaged_player_ptr + 0xC8); // player_ptr/unit_ptr + 0xC8 = damaging player this works on vehicles/AI and such too.
	}

	return -1;
}

int __cdecl call_get_character_sid_from_datum(int char_datum)
{
	typedef int(__cdecl get_character_sid_from_datum)(int datum);
	auto p_get_character_sid_from_datum = h2mod->GetAddress<get_character_sid_from_datum*>(0x31796C);

	return p_get_character_sid_from_datum(char_datum);
}

int __cdecl call_fill_creation_data_from_object_index(int object_index, void* creation_data)
{
	typedef int(__cdecl fill_creation_data_from_object_index)(int datum,void* creation_data);
	auto p_fill_creation_data_from_object_index = h2mod->GetAddress<fill_creation_data_from_object_index*>(0x1F24ED);

	return p_fill_creation_data_from_object_index(object_index, creation_data);
}

signed int __cdecl object_new_hook(ObjectPlacementData* new_object)
{
	int variant_index = *(int*)((char*)new_object + 0xC);
	signed int result = call_object_new(new_object);

	//unsigned __int16 object_index = result & 0xFFFF;

	object_to_variant[result] = variant_index;

	//wchar_t DebugText[255] = { 0 };
	//SecureZeroMemory(DebugText, sizeof(DebugText));
	//wsprintf(DebugText, L"AI object_new hook - object_index: %08X - variant_index: %08X - datum: %08X", result, variant_index);

	//LOG_TRACE_GAME_N("AI object_new hook - object_index: %08X - variant_index: %08X - datum: %08X", result, variant_index);

	//addDebugText(DebugText);

	return result;
}





typedef void(__stdcall *tc_simulation_unit_entity_definition_creation_encode)(void* thisptr, int creation_data_size, void* creation_data, int a3, bitstream* stream);
tc_simulation_unit_entity_definition_creation_encode pc_simulation_unit_entity_definition_encode;

void __stdcall c_simulation_unit_entity_definition_creation_encode(void *thisptr, int creation_data_size, void* creation_data, int a3, bitstream* stream)
{
	//LOG_TRACE_GAME_N("c_simulation_unit_entity_definition_creation_encode()\r\nthisptr: %08X, creation_data_size: %i, creation_data: %08X, a3: %i, packet: %08X", thisptr, creation_data_size, creation_data, a3, packet);
	int object_permutation_index = *(int*)((char*)creation_data + 0x24);
	if (object_permutation_index != -1)
	{
		//LOG_TRACE_GAME_N("creation_data+0x24: %08X", object_permutation_index);

		stream->data_encode_bool("object-permutation-exists", 1);
		stream->data_encode_integer("object-permutation-index", object_permutation_index, 32);
		//LOG_TRACE_GAME_N("c_simulation_unit_entity_encode - object-permutation-exists packet: %08X, *packet: %08X", packet, *(int*)packet);

	}
	else
		stream->data_encode_bool("object-permutation-exists", 0);

	pc_simulation_unit_entity_definition_encode(thisptr, creation_data_size, creation_data, a3, stream);
}


typedef bool(__stdcall *tc_simulation_unit_entity_definition_creation_decode)(void* thisptr, int creation_data_size, void* creation_data, bitstream* stream);
tc_simulation_unit_entity_definition_creation_decode pc_simulation_unit_entity_definition_decode;

bool __stdcall c_simulation_unit_entity_definition_creation_decode(void *thisptr, int creation_data_size, void* creation_data, bitstream* stream)
{
	//LOG_TRACE_GAME_N("c_simulation_unit_entity_definition_creation_decode()\r\nthisptr: %08X, creation_data_size: %i, creation_data: %08X, packet: %08X", thisptr, creation_data_size, creation_data, packet);
	
	if (stream->data_decode_bool("object-permutation-exists"))
	{
		//LOG_TRACE_GAME_N("c_simulation_unit_entity_decode - object-permutation-exists packet: %08X, *packet: %08X", packet, *(int*)packet);
		int object_permutation_index = stream->data_decode_integer("object-permutation-index", 32);
		*(int*)((char*)creation_data + 0x24) = object_permutation_index;

		//LOG_TRACE_GAME_N("object_permutation_index: %08X", object_permutation_index);
	}
	else
		*(int*)((char*)creation_data + 0x24) = -1;

	return pc_simulation_unit_entity_definition_decode(thisptr, creation_data_size, creation_data, stream);
}

typedef int(__stdcall *tset_unit_creation_data)(unsigned int object_index, void* object_creation_data);
tset_unit_creation_data pset_unit_creation_data;

int __stdcall set_unit_creation_data_hook(unsigned int object_index, void* object_creation_data)
{
	int result = pset_unit_creation_data(object_index, object_creation_data);

	if (object_to_variant[object_index] != 0)
	{
		//We should have allocated an additional 4 bytes above 0x24 so we'll write our in between 0x24 and 0x28
		*(int*)((char*)object_creation_data + 0x24) = object_to_variant[object_index];

		/*wchar_t DebugText[255] = { 0 };
		SecureZeroMemory(DebugText, sizeof(DebugText));
		wsprintf(DebugText, L"AI unit_creation_data_setup hook - object_index: %08X - variant_index: %08X", object_index, object_to_variant[object_index]);

		LOG_TRACE_GAME_N("set_unit_creation_data_hook - object_index: %08X, variant_index: %08X", object_index, object_to_variant[object_index]);
		addDebugText(DebugText);*/
	}
	else
		*(int*)((char*)object_creation_data + 0x24) = -1;

	return result;
}

typedef bool(__cdecl *tset_unit_color_data)(int, unsigned __int16, int a3);
tset_unit_color_data pset_unit_color_data;

bool __cdecl set_unit_color_data_hook(int a1, unsigned __int16 a2, int a3)
{
	int object_creation_data = a1 - 0x10;
	int object_permutation_index = *(int*)((char*)object_creation_data + 0x24);

	LOG_TRACE_GAME("set_unit_color_data_hook - {:x}", object_permutation_index);

	if ( object_permutation_index == -1)
		return pset_unit_color_data(a1, a2, a3);

	return 0;
}

typedef bool(__stdcall *tcreate_unit_hook)(void*, int, int, void*);
tcreate_unit_hook pcreate_unit_hook;

bool __stdcall create_unit_hook(void* pCreationData, int a2, int a3, void* pObject)
{
	if (*(int*)((char*)pCreationData + 0x24) != -1)
	{
		//wchar_t DebugText[255] = { 0 };
		//SecureZeroMemory(DebugText, sizeof(DebugText));
		//wsprintf(DebugText, L"create_unit_hook - variant type: %08X - ", *(int*)((char*)pCreationData + 0x24));

		//addDebugText(DebugText);

		*(int*)((char*)pObject + 0xC) = *(int*)((char*)pCreationData + 0x24);
	}

	return pcreate_unit_hook(pCreationData, a2, a3, pObject);
}

wchar_t* H2MOD::get_session_game_variant_name()
{
	return h2mod->GetAddress<wchar_t*>(0x97777C, 0x534A18);
}

void H2MOD::leave_session()
{
	if (h2mod->Server)
		return;

	if (GetMapType() != scnr_type::MainMenu)
	{
		// request_squad_browser
		WriteValue<BYTE>(h2mod->GetAddress(0x978BAC), 1);

		typedef void(__cdecl load_main_menu_with_context)(int context);
		auto load_main_menu_with_context_impl = h2mod->GetAddress<load_main_menu_with_context*>(0x08EAF);
		load_main_menu_with_context_impl(0);
	}

	typedef int(__cdecl leave_game_type)(int a1);
	auto leave_session = h2mod->GetAddress<leave_game_type*>(0x216388);
	leave_session(0);
}

#pragma endregion

typedef int(__cdecl *show_error_screen)(int a1, signed int a2, int a3, __int16 a4, int a5, int a6);
show_error_screen show_error_screen_method;

int __cdecl showErrorScreen(int a1, signed int a2, int a3, __int16 a4, int a5, int a6) {
	if (a2 == 0x117)
	{
		LOG_TRACE_FUNC("Ignoring need to reinstall maps");
		return 0;
	}
	return show_error_screen_method(a1, a2, a3, a4, a5, a6);
}

typedef signed int(__cdecl *wcsncpy_s_hook)(int a1, unsigned int a2, wchar_t* a3, int a4);
wcsncpy_s_hook p_wcsncpy_s_hook;

//lets you follow the call path of any string that is displayed (in a debugger)
signed int __cdecl stringDisplayHook(int a1, unsigned int a2, wchar_t* a3, int a4) {
	return p_wcsncpy_s_hook(a1, a2, a3, a4);
}

/* controller index aka local player index -> player index */
datum H2MOD::get_player_datum_index_from_controller_index(int controller_index) 
{
	typedef int(__cdecl* get_local_player_index)(int controller_index); 
	auto p_get_local_player_index = reinterpret_cast<get_local_player_index>(h2mod->GetAddress(0x5141D));
	return p_get_local_player_index(controller_index); 
}

#pragma region PlayerFunctions

float H2MOD::get_distance(int playerIndex1, int playerIndex2) {
	real_point3d points_distance;
	real_point3d* player1 = nullptr;
	real_point3d* player2 = nullptr;

	player1 = h2mod->get_player_unit_coords(playerIndex1);
	player2 = h2mod->get_player_unit_coords(playerIndex2);
	
	points_distance.x = abs(player1->x - player2->x);
	points_distance.y = abs(player1->y - player2->y);
	points_distance.z = abs(player1->z - player2->z);

	return sqrt(pow(points_distance.x, 2) + pow(points_distance.y, 2) + pow(points_distance.z, 2));
}

real_point3d* H2MOD::get_player_unit_coords(int playerIndex) {
	BYTE* player_unit = get_player_unit_from_player_index(playerIndex);
	if (player_unit != nullptr)
		return reinterpret_cast<real_point3d*>(player_unit + 0x64);

	return nullptr;
}

BYTE* H2MOD::get_player_unit_from_player_index(int playerIndex) {
	datum unit_datum = get_unit_datum_from_player_index(playerIndex);
	if (unit_datum.IsNull())
		return nullptr;

	DatumIterator<ObjectHeader> objectsIt(game_state_objects_header);
	return (BYTE*)objectsIt.get_data_at_index(unit_datum.ToAbsoluteIndex())->object;
}

void call_give_player_weapon(int PlayerIndex, datum WeaponId, bool bReset)
{
	//LOG_TRACE_GAME("GivePlayerWeapon(PlayerIndex: %08X, WeaponId: %08X)", PlayerIndex, WeaponId);

	datum unit_datum = h2mod->get_unit_datum_from_player_index(PlayerIndex);

	if (unit_datum != -1)
	{
		ObjectPlacementData nObject;

		call_object_placement_data_new(&nObject, WeaponId, unit_datum, 0);

		int object_index = call_object_new(&nObject);

		if (bReset == true)
			call_unit_reset_equipment(unit_datum);

		call_assign_equipment_to_unit(unit_datum, object_index, 1);
	}
}

wchar_t* H2MOD::get_local_player_name(int local_player_index)
{
	return this->get_player_name_from_player_index(this->get_player_datum_index_from_controller_index(local_player_index).ToAbsoluteIndex());
}

int H2MOD::get_player_index_from_name(wchar_t* playername)
{
	PlayerIterator playersIt;

	while (playersIt.get_next_player())
	{
		wchar_t* comparename = playersIt.get_current_player_name();

		LOG_TRACE_GAME(L"[H2MOD]::get_player_index_from_name( {0} : {1} )", playername, comparename);

		if (wcscmp(comparename, playername))
		{
			return playersIt.get_current_player_index();
		}
	}
	return -1;
}

wchar_t* H2MOD::get_player_name_from_player_index(int playerIndex)
{
	PlayerIterator playersIt;
	return playersIt.get_data_at_index(playerIndex)->properties.player_name;
}

int H2MOD::get_player_index_from_unit_datum(datum unit_datum_index)
{
	PlayerIterator playersIt;
	while (playersIt.get_next_player())
	{
		datum unit_datum_index_check = playersIt.get_current_player_data()->BipedUnitDatum;
		LOG_TRACE_FUNC("Checking datum: {0:x} - index: {1} against datum: {2:x}", unit_datum_index_check.ToInt(), playersIt.get_current_player_index(), unit_datum_index.ToInt());

		if (unit_datum_index == unit_datum_index_check)
			return playersIt.get_current_player_index();
	}
	return -1;
}

datum H2MOD::get_unit_datum_from_player_index(int playerIndex)
{
	PlayerIterator playersIt;
	if (!playersIt.get_data_at_index(playerIndex)->BipedUnitDatum.IsNull())
		return playersIt.get_data_at_index(playerIndex)->BipedUnitDatum;

	return -1;
}

int H2MOD::get_unit_index_from_player_index(int playerIndex)
{
	PlayerIterator playersIt;
	if (!playersIt.get_data_at_index(playerIndex)->BipedUnitDatum.IsNull())
		return playersIt.get_data_at_index(playerIndex)->BipedUnitDatum.ToAbsoluteIndex();

	return -1;
}

//can be used on clients and server
void H2MOD::set_unit_biped(Player::Biped biped_type, int playerIndex)
{
	PlayerIterator playersIt;
	if (playerIndex >= 0 && playerIndex < 16)
		playersIt.get_data_at_index(playerIndex)->properties.profile.player_character_type = biped_type;


	//*(BYTE*)(((BYTE*)h2mod->GetAddress(0x977104) + 0x20CC * playerIndex) + 8) = (BYTE)biped_type;
}

BYTE H2MOD::get_unit_team_index(datum unit_datum_index)
{
	BYTE tIndex = 0;
	int unit_object = call_object_try_and_get_with_type(unit_datum_index, 3);
	if (unit_object)
	{
		tIndex = *(BYTE*)((BYTE*)unit_object + 0x13C);
	}
	return tIndex;
}

void H2MOD::set_unit_team_index(int unit_datum_index, BYTE team) 
{
	int unit_object = call_object_try_and_get_with_type(unit_datum_index, 3);
	if (unit_object)
	{
		*(BYTE*)((BYTE*)unit_object + 0x13C) = team;
	}
}

void H2MOD::set_unit_speed_patch(bool hackit) {
	//TODO: create a way to undo the patch in the case when more than just infection relies on this.
	//Enable Speed Hacks
	
	BYTE origBytes[8] = { 0 };

	DWORD *patch_addr = h2mod->GetAddress<DWORD*>(0x6AB7F, 0x6A3BA);

	if(origBytes[0] == 0)
		memcpy(origBytes, patch_addr, 8);

	if (hackit == true)
	{
		BYTE assmPatchSpeed[8];
		memset(assmPatchSpeed, 0x90, 8);
		WriteBytes((DWORD)patch_addr, assmPatchSpeed, 8);
	}
	else
	{
		WriteBytes((DWORD)patch_addr, origBytes, 8);
	}

}

void H2MOD::set_unit_speed(float speed, int playerIndex)
{
	PlayerIterator playersIt;
	if (playerIndex >= 0 && playerIndex < 16)
		playersIt.get_data_at_index(playerIndex)->unit_speed = speed;
}

void H2MOD::set_player_unit_grenades_count(int playerIndex, BYTE type, BYTE count, bool resetEquipment)
{
	if (type > GrenadeType::Plasma)
	{
		LOG_TRACE_GAME("[H2MOD] set_player_unit_grenades_count() Invalid argument: type");
		return;
	}

	static std::string grenadeEquipamentTagName[2] =
	{
		"objects\\weapons\\grenade\\frag_grenade\\frag_grenade"
		"objects\\weapons\\grenade\\plasma_grenade\\plasma_grenade"
	};

	datum unit_datum_index = h2mod->get_unit_datum_from_player_index(playerIndex);
	datum grenade_eqip_tag_datum_index = tags::find_tag(blam_tag::tag_group_type::equipment, grenadeEquipamentTagName[type]);

	int unit_object = call_object_try_and_get_with_type(unit_datum_index, 3);
	if (unit_object)
	{
		if (resetEquipment)
			call_unit_reset_equipment(unit_datum_index);

		typedef bool(__cdecl* simulation_is_predicted)();
		auto p_simulation_is_predicted = h2mod->GetAddress<simulation_is_predicted>(0x498B7, 0x42B54);

		// not sure what these flags are, but this is called when picking up grenades
		typedef void(__cdecl* entity_set_unk_flags)(datum objectIndex, int flags);
		auto p_entity_set_unk_flags = h2mod->GetAddress<entity_set_unk_flags>(0x1B6685, 0x1B05B5);

		typedef void(__cdecl* unit_add_grenade_to_inventory_send)(datum unitDatumIndex, datum equipamentTagIndex);
		auto p_unit_add_grenade_to_inventory_send = h2mod->GetAddress<unit_add_grenade_to_inventory_send>(0x1B6F12, 0x1B0E42);

		// send simulation update for grenades if we control the simulation
		if (!p_simulation_is_predicted())
		{
			// set grenade count
			*(BYTE*)((BYTE*)unit_object + 0x252 + type) = count;

			p_entity_set_unk_flags(unit_datum_index, 0x400000); // not sure what this flag is
			p_unit_add_grenade_to_inventory_send(unit_datum_index, grenade_eqip_tag_datum_index);
		}

		LOG_TRACE_GAME("[H2Mod-GunGame] set_player_unit_grenades_count - sending grenade simulation update, playerIndex={0}, peerIndex={1}", playerIndex, NetworkSession::getPeerIndex(playerIndex));
	}

}

BYTE H2MOD::get_local_team_index()
{
	return *h2mod->GetAddress<BYTE*>(0x51A6B4);
}
#pragma endregion

void H2MOD::disable_sound(int sound)
{
	LOG_TRACE_GAME("tag data address: {:x}", tags::get_tag_data());
	switch (sound)
	{
	case SoundType::Slayer:
		LOG_TRACE_GAME("tag data address + 0xd7dfb4 = {:p}", tags::get_tag_data()[0xd7dfb4]);
		*(DWORD*)(&tags::get_tag_data()[0xd7e05c]) = -1;
		*(DWORD*)(&tags::get_tag_data()[0xd7dfb4]) = -1;
		break;

	case SoundType::GainedTheLead:
		*(DWORD*)(&tags::get_tag_data()[0xd7ab34]) = -1;
		*(DWORD*)(&tags::get_tag_data()[0xd7ac84]) = -1;
		break;

	case SoundType::LostTheLead:
		*(DWORD*)(&tags::get_tag_data()[0xd7ad2c]) = -1;
		*(DWORD*)(&tags::get_tag_data()[0xd7add4]) = -1;
		break;

	case SoundType::TeamChange:
		*(DWORD*)(&tags::get_tag_data()[0xd7b9a4]) = -1;
		break;
	}
}

void H2MOD::custom_sound_play(const char* soundName, int delay)
{
	auto playSound = [=]()
	{
		//std::unique_lock<std::mutex> lck(h2mod->sound_mutex);
		std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(delay);

		LOG_TRACE_GAME("[H2MOD-SoundQueue] - attempting to play sound {0} - delaying {1} miliseconds first", soundName, delay);

		if (delay > 0)
			std::this_thread::sleep_until(timePoint);

		PlaySoundA(soundName, NULL, SND_FILENAME | SND_NODEFAULT);
	};

	if (!h2mod->Server)
		std::thread(playSound).detach();
}

void H2MOD::custom_sound_play(const wchar_t* soundName, int delay)
{
	auto playSound = [=]()
	{
		//std::unique_lock<std::mutex> lck(h2mod->sound_mutex);
		std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(delay);

		LOG_TRACE_GAME(L"[H2MOD-SoundQueue] - attempting to play sound {0} - delaying {1} miliseconds first", soundName, delay);

		if (delay > 0)
			std::this_thread::sleep_until(timePoint);

		PlaySound(soundName, NULL, SND_FILENAME | SND_NODEFAULT);
	};

	if (!h2mod->Server)
		std::thread(playSound).detach();
}

typedef char(__cdecl *player_death)(int unit_datum_index, int a2, char a3, char a4);
player_death pplayer_death;

typedef void(__stdcall *update_player_score)(void* thisptr, unsigned short a2, int a3, int a4, int a5, char a6);
update_player_score pupdate_player_score;

/* This is technically closer to object death than player-death as it impacts anything with health at all. */

char __cdecl OnPlayerDeath(int unit_datum_index, int a2, char a3, char a4)
{

	//LOG_TRACE_GAME("OnPlayerDeath(unit_datum_index: %08X, a2: %08X, a3: %08X, a4: %08X)", unit_datum_index,a2,a3,a4);

	/* The first value within a2 ( *(DWORD*)(a2) ) appears to be the datum_index of a player from the gamestate_player_table */

	/* This is the unit of the player who last damaged the object*/
	int damaging_player_unit = get_damage_owner(unit_datum_index);

	ScriptEngine::sqOnPlayerDeath(NULL, unit_datum_index, *(int*)(a2));

	if (b_HeadHunter)
	{
		datum dead_player = datum(unit_datum_index);
		headHunterHandler->playerDeath->SetDeadPlayer(dead_player); // set this so we can spawn a skull on their position.
		headHunterHandler->playerDeath->execute();
	}

	if (b_FireFight)
	{
		/* Hack until rest of code is changed to support datum vs int*/
		datum ai_datum = datum(unit_datum_index);

		/*
			In firefight we want to track AI deaths and execute on them to grant points.
		*/
		fireFightHandler->playerDeath->SetPlayerIndex(*(datum*)(a2)); // this is the player datum of player who killed the datum.
		fireFightHandler->playerDeath->SetKilledDatum(ai_datum);
		fireFightHandler->playerDeath->execute();
	}

	if (b_GunGame) 
	{
		gunGame->playerDeath->setUnitDatumIndex(unit_datum_index);
		gunGame->playerDeath->execute();
	}

	if (b_Infection) {
		infectionHandler->playerDeath->setUnitDatumIndex(unit_datum_index);
		infectionHandler->playerDeath->execute();
	}

	return pplayer_death(unit_datum_index, a2, a3, a4);
}


void __stdcall OnPlayerScore(void* thisptr, unsigned short a2, int a3, int a4, int a5, char a6)
{
	//LOG_TRACE_GAME("update_player_score_hook ( thisptr: %08X, a2: %08X, a3: %08X, a4: %08X, a5: %08X, a6: %08X )", thisptr, a2, a3, a4, a5, a6);
	//20/10/2018 18:46:51.541 update_player_score_hook ( thisptr: 3000595C, a2: 00000000, a3: 00000002, a4: 00000001, a5: 00000007, a6: 00000001 )
	// / 10 / 2018 18:46 : 51.541 update_player_score_hook(thisptr : 3000595C, a2 : 00000000, a3 : 00000000, a4 : 00000001, a5 : FFFFFFFF, a6 : 00000000)
	//	20 / 10 / 2018 18 : 46 : 51.541 update_player_score_hook(thisptr : 3000595C, a2 : 00000001, a3 : 00000003, a4 : 00000001, a5 : 00000009, a6: 00000001)

	//20 / 10 / 2018 18:48 : 39.756 update_player_score_hook(thisptr : 3000595C, a2 : 00000001, a3 : 00000002, a4 : 00000001, a5 : 00000007, a6 : 00000001)
	//	20 / 10 / 2018 18 : 48 : 39.756 update_player_score_hook(thisptr : 3000595C, a2 : 00000001, a3 : 00000000, a4 : 00000001, a5 : FFFFFFFF, a6 : 00000000)
	//	20 / 10 / 2018 18 : 48 : 39.756 update_player_score_hook(thisptr : 3000595C, a2 : 00000000, a3 : 00000003, a4 : 00000001, a5 : 00000009, a6: 00000001)

	if (!ScriptEngine::sqOnUpdateScore(NULL, a2, a5))
		return;

	if (a5 == -1)
	{
		if(b_HeadHunter)
			return;
	}

	if (a5 == 7) //player got a kill?
	{
		int PlayerIndex = a2;

		if (b_GunGame) {
			gunGame->playerKill->setPlayerIndex(PlayerIndex);
			gunGame->playerKill->execute();
		}

		if (b_HeadHunter)
		{
			return;
		}
	}

	pupdate_player_score(thisptr, a2, a3, a4, a5, a6);
}

void H2MOD::toggle_weapon_pickup(bool bEnable)
{
	BYTE orig_bytes[5] = { 0xE8, 0x18, 0xE0, 0xFF, 0xFF };
	DWORD offset = h2mod->GetAddress(0x55EFA);

	if(bEnable)
		WriteBytes(offset, orig_bytes, 5);
	else
		NopFill(offset, 5);

}

void H2MOD::toggle_weapon_pickup()
{

	//Client Sided Patch
	BYTE orig_bytes[5] = { 0xE8, 0x18, 0xE0, 0xFF, 0xFF };

	DWORD offset = h2mod->GetAddress(0x55EFA);
	if (*(BYTE*)offset == 0x90)
		WriteBytes(offset, orig_bytes, 5);
	else
		NopFill(offset, 5);

}

int OnAutoPickUpHandler(datum player_datum, datum object_datum)
{
	int(_cdecl* AutoHandler)(datum, datum);
	AutoHandler = (int(_cdecl*)(datum, datum))h2mod->GetAddress(0x57AA5, 0x5FF9D);

	if (!ScriptEngine::sqOnAutoPickup(NULL,player_datum.ToInt(), object_datum.ToInt()))
		return 0;

	if (b_HeadHunter)
	{
		headHunterHandler->itemInteraction->SetPlayerIndex(player_datum);
		bool handled = headHunterHandler->itemInteraction->SetInteractedObject(object_datum);
		headHunterHandler->itemInteraction->execute();

		if (handled)
			return 0;
	}

	return AutoHandler(player_datum, object_datum);
}

void get_object_table_memory()
{
	game_state_actors = *h2mod->GetAddress<s_datum_array**>(0xA965DC, 0x9A1C5C);
	game_state_players = *h2mod->GetAddress<s_datum_array**>(0x4A8260, 0x4D64C4);
	game_state_objects_header = *h2mod->GetAddress<s_datum_array**>(0x4E461C, 0x50C8EC);
}


typedef bool(__cdecl *map_cache_load)(game_engine_settings* map_load_settings);
map_cache_load p_map_cache_load;

bool __cdecl OnMapLoad(game_engine_settings* engine_settings)
{

	bool result = p_map_cache_load(engine_settings);
	if (result == false) // verify if the game didn't fail to load the map
		return false;
	
	h2mod->SetMapType(engine_settings->map_type);

	tags::run_callbacks();

	get_object_table_memory();

	H2Tweaks::setFOV(H2Config_field_of_view);
	H2Tweaks::setCrosshairPos(H2Config_crosshair_offset);
	H2Tweaks::setVehicleFOV(H2Config_vehicle_field_of_view);

	ScriptEngine::sqOnMapLoad();
	
	if (h2mod->GetMapType() == scnr_type::MainMenu)
	{
		addDebugText("Map Type: Main-Menu");
		object_to_variant.clear();

		//TODO: issue #232
		/*if (!NetworkSession::localPeerIsSessionHost()) {
			advLobbySettings->resetLobbySettings();
		}*/

		if (b_Halo2Final && !h2mod->Server) {
			h2f->Dispose();
			b_Halo2Final = false;
		}

		if (b_Infection) {
			infectionHandler->deinitializer->execute();
			b_Infection = false;
		}

		if (b_GunGame) {
			gunGame->deinitializer->execute();
			b_GunGame = false;
		}

		if (b_XboxTick) {
			engine_settings->tickrate = XboxTick::setTickRate(false);
			b_XboxTick = false;
		}

		UIRankPatch();
		H2Tweaks::setHz();
		H2Tweaks::toggleAiMp(false);
		H2Tweaks::toggleUncappedCampaignCinematics(false);

		return result;
	}		

	wchar_t* variant_name = h2mod->get_session_game_variant_name();
	LOG_INFO_GAME(L"[h2mod] OnMapLoad map type {0}, variant name {1}", h2mod->GetMapType(), variant_name);
	BYTE GameState = *h2mod->GetAddress<BYTE*>(0x420FC4, 0x3C40AC);

	for (auto gametype_it : GametypesMap)
		gametype_it.second = false; // reset custom gametypes state

	H2Tweaks::setSavedSens();

	if (h2mod->GetMapType() == scnr_type::Multiplayer)
	{
		addDebugText("Map type: Multiplayer");

		for (auto gametype_it : GametypesMap)
		{
			if (StrStrIW(variant_name, gametype_it.first)) {
				LOG_INFO_GAME(L"[h2mod] {1} custom gametype turned on!", gametype_it.first);
				gametype_it.second = true; // enable a gametype if substring is found
			}
		}

		if (!b_XboxTick) 
		{
			HitFix::Initialize();
			H2X::Initialize(b_H2X);
			MPMapFix::Initialize();
			H2Tweaks::applyMeleePatch(true);
			engine_settings->tickrate = XboxTick::setTickRate(false);
		}
		else
		{
			H2Tweaks::applyMeleePatch(false);
			engine_settings->tickrate = XboxTick::setTickRate(true);
		}
		
		H2Tweaks::toggleAiMp(true);
		H2Tweaks::toggleUncappedCampaignCinematics(false);

		H2Tweaks::setCrosshairSize(0, false);
		//H2Tweaks::applyShaderTweaks(); 

		if (GameState == 3)
		{
			// send server map checksums to client
			//MapChecksumSync::SendState();
			//inform players of the current advanced lobby settings
			// TODO: issue #232
			//advLobbySettings->sendLobbySettingsPacket();

			if (b_Infection) {
				infectionHandler->initializer->execute();
			}

			if (b_GunGame) {
				gunGame->initializer->execute();
			}

			if (b_Halo2Final)
				h2f->Initialize();
		}

	}

	else if (h2mod->GetMapType() == scnr_type::SinglePlayer)
	{
		//if anyone wants to run code on map load single player
		addDebugText("Map type: Singleplayer");
		//H2X::Initialize(true);
		H2Tweaks::applyMeleePatch(true);
		H2Tweaks::toggleUncappedCampaignCinematics(true);
	}

	return result;
}

typedef bool(__cdecl *player_spawn)(datum playerDatumIndex);
player_spawn p_player_spawn;

bool __cdecl OnPlayerSpawn(datum playerDatumIndex)
{

	ScriptEngine::sqPrePlayerSpawn(NULL,playerDatumIndex.ToInt());


	//I cant find somewhere to put this where it actually works (only needs to be done once on map load). It's only a few instructions so it shouldn't take long to execute.
	H2Tweaks::toggleKillVolumes(!AdvLobbySettings_disable_kill_volumes);


	if (b_Infection) {
		infectionHandler->preSpawnPlayer->setPlayerIndex(playerDatumIndex.ToAbsoluteIndex());
		infectionHandler->preSpawnPlayer->execute();
	}

	if (b_GunGame) {
		gunGame->preSpawnPlayer->setPlayerIndex(playerDatumIndex.ToAbsoluteIndex());
		gunGame->preSpawnPlayer->execute();
	}

	bool ret = p_player_spawn(playerDatumIndex);
	
	ScriptEngine::sqPostPlayerSpawn(NULL, playerDatumIndex.ToInt());

	if (b_Infection) {
		infectionHandler->spawnPlayer->setPlayerIndex(playerDatumIndex.ToAbsoluteIndex());
		infectionHandler->spawnPlayer->execute();
	}

	if (b_GunGame) {
		gunGame->spawnPlayer->setPlayerIndex(playerDatumIndex.ToAbsoluteIndex());
		gunGame->spawnPlayer->execute();
	}

	return ret;
}

typedef int(__cdecl *object_p_hook)(int s_object_placement_data, int a2, signed int a3, int a4);
object_p_hook object_p_hook_method;

int __cdecl objectPHook(int s_object_placement_data, int object_definition_index, int object_owner, int unk) {
	if (h2mod->hookedObjectDefs.find(object_definition_index) == h2mod->hookedObjectDefs.end()) {
		//ingame only
		wchar_t* mapName = h2mod->GetAddress<wchar_t*>(0x97737C);
		LOG_TRACE_GAME(L"MapName={0}, object_definition_index: {1:x}", mapName, object_definition_index);
		h2mod->hookedObjectDefs.insert(object_definition_index);
	}
	return object_p_hook_method(s_object_placement_data, object_definition_index, object_owner, unk);
}

typedef BOOL(__stdcall *is_debugger_present)();
is_debugger_present is_debugger_present_method;

BOOL __stdcall isDebuggerPresent() {
	return false;
}

typedef void*(__stdcall *tload_wgit)(void* thisptr, int a2, int a3, int a4, unsigned short a5);
tload_wgit pload_wgit;

void* __stdcall OnWgitLoad(void* thisptr, int a2, int a3, int a4, unsigned short a5) {
	int wgit = a2;

	//char NotificationPlayerText[20];
	//sprintf(NotificationPlayerText, "WGIT ID: %d", a2);
	//addDebugText(NotificationPlayerText);
	//MessageBoxA(NULL, NotificationPlayerText, "WGITness", MB_OK);

	pload_wgit(thisptr, wgit, a3, a4, a5);
	return thisptr;
}

typedef void(__cdecl *change_team)(int a1, int a2);
change_team p_change_local_team;

void __cdecl changeTeam(int localPlayerIndex, int teamIndex) 
{
	if (!ScriptEngine::sqOnTeamChange(NULL,localPlayerIndex,teamIndex))
		return;


	network_session* session = NetworkSession::getCurrentNetworkSession();
	if ((session->parameters.field_8 == 4 && get_game_life_cycle() == life_cycle_pre_game)
		|| (StrStrIW(h2mod->get_session_game_variant_name(), L"rvb") != NULL && teamIndex != 0 && teamIndex != 1)) {
		//rvb mode enabled, don't change teams
		return;
	}
	p_change_local_team(localPlayerIndex, teamIndex);
}

void H2MOD::set_local_team_index(int local_player_index, int team_index)
{
	// we only use player index 0 due to no splitscreen support but whatever
	typedef void(__cdecl update_player_profile)(int local_player_index);
	auto p_update_player_profile = h2mod->GetAddress<update_player_profile*>(0x206A97);

	p_change_local_team(local_player_index, team_index);
	p_update_player_profile(local_player_index); // fixes infection handicap glitch
}

void __cdecl print_to_console(char *output)
{
	const static std::string prefix = "[HSC Print] ";
	commands->display(prefix + output);
}

void DuplicateDataBlob(DATA_BLOB  *pDataIn, DATA_BLOB  *pDataOut)
{
	pDataOut->cbData = pDataIn->cbData;
	pDataOut->pbData = static_cast<BYTE*>(LocalAlloc(LMEM_FIXED, pDataIn->cbData));
	CopyMemory(pDataOut->pbData, pDataIn->pbData, pDataIn->cbData);
}

BOOL WINAPI CryptProtectDataHook(
	_In_       DATA_BLOB                 *pDataIn,
	_In_opt_   LPCWSTR                   szDataDescr,
	_In_opt_   DATA_BLOB                 *pOptionalEntropy,
	_Reserved_ PVOID                     pvReserved,
	_In_opt_   CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct,
	_In_       DWORD                     dwFlags,
	_Out_      DATA_BLOB                 *pDataOut
)
{
	DuplicateDataBlob(pDataIn, pDataOut);

	return TRUE;
}

BOOL WINAPI CryptUnprotectDataHook(
	_In_       DATA_BLOB                 *pDataIn,
	_Out_opt_  LPWSTR                    *ppszDataDescr,
	_In_opt_   DATA_BLOB                 *pOptionalEntropy,
	_Reserved_ PVOID                     pvReserved,
	_In_opt_   CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct,
	_In_       DWORD                     dwFlags,
	_Out_      DATA_BLOB                 *pDataOut
)
{
	if (CryptUnprotectData(pDataIn, ppszDataDescr, pOptionalEntropy, pvReserved, pPromptStruct, dwFlags, pDataOut) == FALSE) {
		DuplicateDataBlob(pDataIn, pDataOut); // if decrypting the data fails just assume it's unencrypted
	}

	return TRUE;
}

char filo_write__encrypted_data_hook(filo *file_ptr, DWORD nNumberOfBytesToWrite, LPVOID lpBuffer)
{
	DWORD file_size = GetFileSize(file_ptr->handle, NULL);

	if (file_size > nNumberOfBytesToWrite) // clear the file as unencrypted data is shorter then encrypted data.
		FiloInterface::change_size(file_ptr, 0);
	return FiloInterface::write(file_ptr, lpBuffer, nNumberOfBytesToWrite);
}

DWORD calculate_model_lod;
DWORD calculate_model_lod_detour_end;
__declspec(naked) void calculate_model_lod_detour()
{
	__asm
	{
		// replaced code
		// todo check if this is needed when using a static LOD, might save on some processor time
		call calculate_model_lod

		cmp H2Config_static_lod_state, 0
		jz END_DETOUR

		mov eax, H2Config_static_lod_state
		sub eax, 1 // convert setting to in-game model LOD value (0 - 5, L1 - L6)

		END_DETOUR:
		jmp calculate_model_lod_detour_end
	}
}

typedef void(__cdecl *on_custom_map_change)(const void* a1);
on_custom_map_change on_custom_map_change_method;

void __cdecl onCustomMapChange(const void* a1) {
	on_custom_map_change_method(a1);
}

typedef bool(__cdecl *tfn_c000bd114)(int);
tfn_c000bd114 pfn_c000bd114;
bool __cdecl fn_c000bd114_IsSkullEnabled(int skull_index)
{
	//bool result = pfn_c000bd114(skull_index);
	//return result;

	bool* var_c004d8320 = h2mod->GetAddress<bool*>(0x4d8320);
	bool(*fn_c00049833_IsEngineModeCampaign)() = (bool(*)())h2mod->GetAddress(0x49833);

	bool result = false;
	if (skull_index <= 0xE && fn_c00049833_IsEngineModeCampaign())
		result = var_c004d8320[skull_index];
	if (skull_index == 0xA && AdvLobbySettings_mp_sputnik)
		result = true;
	else if (skull_index == 0x1 && AdvLobbySettings_mp_grunt_bday_party)
		result = true;
	else if (skull_index == 0x6 && AdvLobbySettings_mp_blind)
		result = true;
	return result;
}

bool GrenadeChainReactIsEngineMPCheck() {
	if (AdvLobbySettings_grenade_chain_react)
		return false;
	return h2mod->GetMapType() == scnr_type::Multiplayer;
}

bool BansheeBombIsEngineMPCheck() {
	if (AdvLobbySettings_banshee_bomb)
		return false;
	return h2mod->GetMapType() == scnr_type::Multiplayer;
}

bool FlashlightIsEngineSPCheck() {
	if (AdvLobbySettings_flashlight)
		return true;
	return h2mod->GetMapType() == scnr_type::SinglePlayer;
}

typedef bool(__cdecl* verify_game_version_on_join)(int executable_version, int build_version, int build_version2);
verify_game_version_on_join p_verify_game_version_on_join;

bool __cdecl VerifyGameVersionOnJoin(int executable_version, int build_version, int build_version2)
{
	return executable_version == EXECUTABLE_VERSION && build_version >= GAME_BUILD && build_version2 <= GAME_BUILD;
}

typedef bool(__cdecl* verify_executable_version)(int executable_version);
verify_executable_version p_verify_executable_version;

bool __cdecl VerifyExecutableVersion(int executable_version)
{
	return executable_version == EXECUTABLE_VERSION; // will not display servers that don't match this in server list
}

typedef void(__cdecl *get_game_version)(DWORD *executable_version, DWORD *build_version, DWORD *build_version2);
get_game_version p_get_game_version;

void __cdecl GetGameVersion(DWORD *executable_version, DWORD *build_version, DWORD *build_version2)
{
	*executable_version = EXECUTABLE_VERSION;
	*build_version = GAME_BUILD;
	*build_version2 = GAME_BUILD;
}

void GivePlayerWeaponDatum(datum unit_datum, datum weapon_datum)
{
	if (unit_datum != -1 && unit_datum != 0)
	{
		ObjectPlacementData nObject;

		call_object_placement_data_new(&nObject, weapon_datum, unit_datum, 0);

		int object_index = call_object_new(&nObject);
		if (object_index != -1)
		{
			call_unit_reset_equipment(unit_datum);
			call_assign_equipment_to_unit(unit_datum, object_index, 1);
		}
	}
}

//This is used for maps with 'shops' where the device_acceleration_scale is an indicator that they're using the control device as a 'shop'
float get_device_acceleration_scale(datum device_datum)
{
	DWORD tag_data = (DWORD)tags::get_tag_data();
	DWORD tag_instances = (DWORD)tags::get_tag_instances();
	DWORD game_state_objects_header_table = *(DWORD*)((BYTE*)game_state_objects_header + 0x44);

	int device_gamestate_offset = device_datum.Index + device_datum.Index * 2;
	DWORD device_gamestate_datum_pointer = *(DWORD*)((BYTE*)game_state_objects_header_table + device_gamestate_offset * 4 + 8);
	DWORD device_control_datum = *(DWORD*)((BYTE*)device_gamestate_datum_pointer);

	__int16 device_control_index = device_control_datum & 0xFFFF;
	device_control_index = device_control_index << 4;

	DWORD device_control_tag_offset = *(DWORD*)((BYTE*)device_control_index + tag_instances + 8);
	float acceleration_scale = *(float*)((BYTE*)device_control_tag_offset + tag_data + 0x14);

	return acceleration_scale;

}

typedef int(__cdecl *tdevice_touch)(datum device_datum, datum unit_datum);
tdevice_touch pdevice_touch;

bool device_active = true;
//This happens whenever a player activates a device control.
int __cdecl device_touch(datum device_datum, datum unit_datum)
{
	if (h2mod->GetMapType() == scnr_type::Multiplayer)
	{
		//We check this to see if the device control is a 'shopping' device, if so send a request to buy an item to the DeviceShop.
		if (get_device_acceleration_scale(device_datum) == 999.0f)
		{
			if (device_shop->BuyItem(device_datum, unit_datum)) // If the purchase was successful we won't execute the original device control action.
			{
				if (device_active == false)
					return pdevice_touch(device_datum, unit_datum);

				device_active = true;
				return 0;
			}
			// If the purchase fails (they don't have enough points), or the device is not a shopping device return normally.
			// In general's map returning normally will turn the point display red indicating the user has no points, we do not indicate that the purchase failed in any other way.
			if (device_active == false)
				return 0;

			device_active = false;
		}
	}

	return pdevice_touch(device_datum, unit_datum);
}

/*
	A handler for XLiveSessionEnd to detect when players are leaving games.
*/
void H2MOD::session_end()
{
	ScriptEngine::sqSessionEnd();
}

#pragma pack(push, 1)
struct perma_struct
{
	void *__vftable;
	int field_4;
	BYTE gap_8[4];
	DWORD field_C;
	BYTE gap_10[96];
	s_datum_array *field_70;
	BYTE gap_74[56];
	int field_AC;
	char field_B0;
	BYTE gap_B1[923];
	char field_44C;
	char field_44D;
	char field_44E;
	char field_44F;
	char field_450;
	BYTE gap_451[3];
	void *__vftable_454;
	DWORD field_458;
	BYTE gap_45C[8];
	char *field_464;
	void(__thiscall *field_468)(void **pthis, int a2, int *a3);
};
#pragma pop(push)

typedef char(__stdcall* tc_game_engine_category_list_create_labels)(void* pThis, int a2, int a3);
tc_game_engine_category_list_create_labels pc_game_engine_category_list_create_labels;

char __stdcall c_game_engine_category_list_create_labels(void* pThis, int a2, int a3)
{
	char(__thiscall* c_list_create_labels)(void*, int, int, int, int*, int) = (char(__thiscall*)(void*, int, int, int, int*, int))h2mod->GetAddress(0x002139f8);

	int label_list[16];
	label_list[0] = 0;
	label_list[1] = 0x600010F;
	label_list[2] = 1;
	label_list[3] = 0x4000238;
	label_list[4] = 2;
	label_list[5] = 0x7000110;
	label_list[6] = 3;
	label_list[7] = 0xA000113;
	label_list[8] = 4;
	label_list[9] = 0x3000437;
	label_list[10] = 5;
	label_list[11] = 0x700010E;
	label_list[12] = 6;
	label_list[13] = 0xB000115;
	label_list[14] = 7;
	label_list[15] = 0x0E0006D9;

	return c_list_create_labels(pThis, a2, a3, 0, label_list, 8);
	//return pc_game_engine_category_list_create_labels(pThis, a2, a3);
	return 1;

}

typedef void (__stdcall* tConVariantCategoryList)(perma_struct *pThis, int a2);
tConVariantCategoryList pConVariantCategoryList;

void __stdcall ConVariantCategoryList(perma_struct* pThis, int a2)
{

	typedef WORD(__cdecl* tget_free_datum)(void* a1);
	auto get_free_datum = h2mod->GetAddress<tget_free_datum>(0x667A0);

	WriteValue<BYTE>(h2mod->GetAddress(0x249D9D), 0x08);
	WriteValue<BYTE>(h2mod->GetAddress(0x249DFF), 0x08);
	
	pConVariantCategoryList(pThis, a2);
	
	*&pThis->field_70->datum[4 * (WORD)get_free_datum(pThis->field_70) + 2] = 7;

}

typedef char (__stdcall *tvariant_list_button_handler)(DWORD* pThis, int *a1, int *a3);
tvariant_list_button_handler pvariant_list_button_handler;

char __stdcall variant_list_button_handler(DWORD* pThis, int* a2, int *a3)
{

	ScriptEngine::sqSessionEnd();

	int button_index = *a3;
	DWORD button_pointer = *(DWORD*)(pThis[0x1C] + 0x44) + 12 * (WORD)button_index;
	signed int button_datum = *(DWORD*)(button_pointer + 4);

	DWORD *SaveGameBuffer = h2mod->GetAddress<DWORD*>(0x482300);
	if (!(((unsigned int)button_datum >> 21) & 1) && *SaveGameBuffer)
	{
		int variant_index = button_datum >> 8 & 0x1FFF;
		DWORD* variant_buff = (DWORD*)(0x234004 * ((button_datum >> 4) & 0xF) + *(DWORD*)SaveGameBuffer + 0x71EC0); // get variant count
		int variant_count = *variant_buff - 1; // subtract variant count
		if (variant_index <= variant_count || variant_count == variant_index)
		{
			DWORD *variant_pointer = &variant_buff[0x8D * variant_index + 1];

			if (StrStrW((wchar_t*)variant_pointer, L"Scripts\\Variants"))
			{

				std::wstring variant_path = std::wstring((wchar_t*)variant_pointer);
				variant_path = variant_path.substr(variant_path.find_first_of(L":\\")+2, variant_path.length()); // Remove C:/
				variant_path = variant_path.substr(0,variant_path.find_last_of(L"\\"));
				variant_path.append(L".nut");
				std::wstring_convert<std::codecvt_utf8<wchar_t>> string_to_wstring;
				std::string script_path = string_to_wstring.to_bytes(variant_path);

				ScriptEngine::sqSessionStart();
				ScriptEngine::sqLoadScript(script_path);
		
				return pvariant_list_button_handler(pThis, a2, a3);
			}
		}
	}
	return pvariant_list_button_handler(pThis, a2, a3);
}

#pragma region SCRIPT_LOAD_FUNCTION
/*
	This is where we'll check for the existence of scripts and create variants in relation to them.
*/
struct variant_memory {
	wchar_t path[0x104];
	wchar_t name[0x12];
	BYTE index;
	char pad[0x07];
};

std::map<std::wstring, BYTE> variant_paths
{
	{ L"Slayer", 1},
	{ L"KOTH", 2},
	{ L"Oddball", 4},
	{ L"Juggernaut", 5},
	{ L"CTF", 7},
	{ L"Assault", 8},
	{ L"Territories", 9},
};

std::map<std::wstring, BYTE> variant_engine
{
	{L"ctf",1},
	{L"slayer", 2},
	{L"oddball",3},
	{L"koth",4},
	{L"juggernaut",7},
	{L"territories",8},
	{L"assault",9}
};


void InjectVariant(std::wstring variant_name, BYTE index,std::wstring engine_name)
{
	BYTE* variant_address = ((*(BYTE**)(h2mod->GetAddress<BYTE*>(0x482300))) + 0x71EC0);
	int variant_count = *(int*)variant_address;

	FILE *f = std::fopen("Scripts\\binary\\variant_dump.bin", "r+");
	fread(variant_address + 4 + (0x234 * variant_count), 0x234, 1, f);
	fclose(f);
	*(int*)variant_address = variant_count + 1;

	variant_memory *variant = (variant_memory*)((BYTE*)variant_address + 4 + (0x234 * variant_count));
	variant_name = variant_name.substr(0, variant_name.length() - 4);

	for (int i = 0; i < variant_count; i++)
	{
		variant_memory *variant = (variant_memory*)((BYTE*)variant_address + 4 + (0x234 * i));
		//wchar_t* dbgmsg = new wchar_t[255];
		//wsprintf(dbgmsg, L"Variant Name: %s, Variant_index: %i", variant->name, variant->index);
		//MessageBoxW(NULL, dbgmsg, L"Debug Data", MB_OK);
		//MessageBoxW(NULL, variant->name, L"Variant", MB_OK);

		//if (variant->index == 7)
		//{
		//	FILE *fw = std::fopen("Scripts\\binary\\ctf_dump.bin", "w");
		//	fwrite(variant, 0x234, 1, fw);
		//	fclose(fw);
		//	MessageBoxA(NULL, "Dumped", "Dumped", MB_OK);
		//}
	}

	wchar_t *var_name_mod = new wchar_t[variant_name.length()+3];
	wmemset(var_name_mod, 0x00, variant_name.length() + 3);
	wsprintf(var_name_mod, L"%ws ",variant_name.c_str());
	wmemset((var_name_mod + variant_name.length()+1), 0xE007, 1);

	wsprintf(variant->name, var_name_mod);
	wsprintf(variant->path, L"C:\\Scripts\\Variants\\%ws\\%ws\\", engine_name.c_str(), variant_name.c_str());
	variant->index = index;
}

void SearchForVariants(std::wstring path)
{
	WIN32_FIND_DATA data;
	HANDLE hFind; 

	std::wstring variant_type = path.substr(17, path.length());

	path.append(L"\\*.nut");
	if ((hFind = FindFirstFile(path.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do {
			InjectVariant(data.cFileName, variant_paths[variant_type],variant_type);
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	path.clear();
}



void LoadSquirrelVariants()
{
	for (auto it = begin(variant_paths); it != end(variant_paths); it++)
	{
		std::wstring path(L"Scripts\\Variants\\");
		path.append(it->first);
		SearchForVariants(path);
	}
}
#pragma endregion

#pragma region SAVE_GAME_LOAD
/*
	Responsible for opening variant and save data and loading it into memory.
	We're placing a hook here to add custom variants based on scripts using generic data.
*/
struct SaveStackData {
	char Pad[2]; //0x00
	bool FileBool; // 0x02
	char Pad2[1]; // 0x03
	int FileState; // 0x04
	char FilePath[255]; // 0x08
};

struct SaveStackStruct {
	void* OutBuffer;
	int size; // 0x08
	char pad2[0x04]; // 0x0C
	struct SaveStackData *pSaveStackData;
};

typedef int(__cdecl *tLoadSaveGame)(int a1,SaveStackStruct *a2,char);
tLoadSaveGame pLoadSaveGame;

int __cdecl LoadSaveGameHook(int a1, SaveStackStruct *a2, char a3)
{

	if (strstr(a2->pSaveStackData->FilePath, "Scripts\\Variants"))
	{


		std::wstring_convert<std::codecvt_utf8<wchar_t>> string_to_wstring;
		std::wstring widepath = string_to_wstring.from_bytes(std::string(a2->pSaveStackData->FilePath));
		std::wstring engine_name = widepath.substr(widepath.find_last_of(L"\\", widepath.length()) + 1);
		std::wstring clean_path = widepath.substr(0, widepath.find_last_of(L"\\"));
		std::wstring variant_name = clean_path.substr(clean_path.find_last_of(L"\\") + 1, clean_path.length());

		std::string dummy_file = "Scripts\\binary\\";
		dummy_file.append(string_to_wstring.to_bytes(engine_name));
		dummy_file.append(".bin");

		FILE* f = std::fopen(dummy_file.c_str(), "r+");
		//FILE* f = std::fopen("Scripts\\binary\\slayer.bin", "r+");
		fread(a2->OutBuffer, 304, 1, f);
		fclose(f);

		a2->pSaveStackData->FileBool = 1;
		a2->pSaveStackData->FileState = 0;

		//std::wstring variant_name = widepath.substr(17, widepath.length() - 17 - engine_name.length() - 1);

		wchar_t *var_name_mod = new wchar_t[variant_name.length() + 3];
		wmemset(var_name_mod, 0x00, variant_name.length() + 3);
		wsprintf(var_name_mod, L"%ws  ", variant_name.c_str());

		wsprintf((wchar_t*)((BYTE*)a2->OutBuffer + 4), var_name_mod);
		*(BYTE*)((BYTE*)a2->OutBuffer + 0x44) = variant_engine[engine_name];

		short flags = *(short*)((BYTE*)a2->OutBuffer + 0x48);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sqTeamGame ^ flags) & (1UL << variant_flag_bitfield::_game_engine_teams_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sqMotionSensor ^ flags) & (1UL << variant_flag_bitfield::_game_engine_motion_sensor_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgAlwaysInvisible ^ flags) & (1UL << variant_flag_bitfield::_game_engine_always_invisible_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgRoundSwitchResetsMap ^ flags) & (1UL << variant_flag_bitfield::_game_engine_round_switch_resets_map_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgObservers ^ flags) & (1UL << variant_flag_bitfield::_game_engine_observers_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgAllowChangingTeams ^ flags) & (1UL << variant_flag_bitfield::_game_engine_changing_teams_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgFriendlyFire ^ flags) & (1UL << variant_flag_bitfield::_game_engine_friendly_fire_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgOvershieldsOnMap ^ flags) & (1UL << variant_flag_bitfield::_game_engine_overshields_on_map_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgInvisibilityOnMap ^ flags) & (1UL << variant_flag_bitfield::_game_engine_invisiblity_on_map_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgGrenadesOnMap ^ flags) & (1UL << variant_flag_bitfield::_game_engine_grenades_on_map_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgStartingGrenades ^ flags) & (1UL << variant_flag_bitfield::_game_engine_starting_grenades_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgExtraDamage ^ flags) & (1UL << variant_flag_bitfield::_game_engine_extra_damage_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgDamageResistant ^ flags) & (1UL << variant_flag_bitfield::_game_engine_extra_damage_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgForceEvenTeams ^ flags) & (1UL << variant_flag_bitfield::_game_engine_force_even_teams_bit);
		*(short*)((BYTE*)a2->OutBuffer + 0x48) ^= (-(unsigned long)ScriptEngine::g_sgRoundSetting1Round ^ flags) & (1UL << variant_flag_bitfield::_game_engine_round_setting_1_round);


		*(short*)((BYTE*)a2->OutBuffer + 0x54) = ScriptEngine::g_sqRoundTime;

		return 1;
	}

	return pLoadSaveGame(a1,a2,a3);
}
#pragma endregion

#pragma region SAVE_GAME_READ_TO_MEMORY
/* 
	Reads data from memory from one location to another related to save game information.
	Save Game information includes profiles and custom variants.

	- We're hooking here so we can load scripts as variants after the initial custom variants have been loaded.
*/

typedef char (__cdecl*tReadVariantToMemory)(DWORD a1);
tReadVariantToMemory pReadVariantToMemory;

char __cdecl ReadVariantToMemory(DWORD a1)
{
	char ret = pReadVariantToMemory(a1);
	LoadSquirrelVariants();
	return ret;
}
#pragma endregion

void H2MOD::team_player_indicator_visibility(bool toggle)
{
	this->drawTeamIndicators = toggle;
}

/*
	Seems to be a function responsible for loading data about maps when displaying them.
	This is hooked to fix/re-add removed custom map images.
*/
DWORD ret_addr;
void __declspec(naked) load_map_data_for_display() {
	__asm {
		pop ret_addr
		mov eax,[esp+0x0C] // grab map_data pointer from stack
		mov ecx,[eax+0x964] // mov bitmap pointer into ecx
		mov [ebx],ecx // mov bitmap pointer into map_data on stack
		push 0
		push 0x9712C8 // original data to be loaded back to eax, (replaces the original function call)
		mov ecx,h2mod
		call H2MOD::GetAddress // get data original function got, and return
		push ret_addr // push return address to stack
		ret // return to original load_map_data_for display function
	
	}
}

void __cdecl game_mode_engine_draw_team_indicators()
{
	typedef void(__cdecl* game_mode_engine_draw_team_indicators_def)();
	auto p_game_mode_engine_draw_team_indicators = h2mod->GetAddress<game_mode_engine_draw_team_indicators_def>(0x6AFA4);

	if (h2mod->drawTeamIndicators)
		p_game_mode_engine_draw_team_indicators();
}

bool __cdecl game_is_minimized_hook()
{
	// if xbox tickrate is set, use the built in frame limiter
	if (b_XboxTick)
		return true;

	// otherwise never use this frame limiter
	return false;
}


void H2MOD::ApplyUnitHooks()
{
	// increase the size of the unit entity creation definition packet
	WriteValue<DWORD>(h2mod->GetAddress(0x1F8028, 0x1E1D8E) + 1, 48);

	//This encodes the unit creation packet, only gets executed on host.
	pc_simulation_unit_entity_definition_encode = (tc_simulation_unit_entity_definition_creation_encode)DetourClassFunc(h2mod->GetAddress<BYTE*>(0x1F8503, 0x1E2269), (BYTE*)c_simulation_unit_entity_definition_creation_encode, 10);

	//This decodes the unit creation packet, only gets executed on client.
	pc_simulation_unit_entity_definition_decode = (tc_simulation_unit_entity_definition_creation_decode)DetourClassFunc(h2mod->GetAddress<BYTE*>(0x1F8557, 0x1E22BD), (BYTE*)c_simulation_unit_entity_definition_creation_decode, 11);

	pdevice_touch = (tdevice_touch)DetourFunc(h2mod->GetAddress<BYTE*>(0x163420, 0x158EE3), (BYTE*)device_touch, 10);

	//Only patch the object_new call on host during AI_Place function, no reason to hook all object_new calls.
	PatchCall(h2mod->GetAddress(0x318DEC, 0x2C3B56), object_new_hook);

	//We update creation data here which is used later on to add data to the packet
	PatchCall(h2mod->GetAddress(0x1F807A, 0x1E1DE0), set_unit_creation_data_hook);
	pset_unit_creation_data = h2mod->GetAddress<tset_unit_creation_data>(0x1F24ED, 0x1DD586);

	// Hooks a call within the creat_unit property on the client side in order to set their permutation index before spawn.
	PatchCall(h2mod->GetAddress(0x1F9E6C, 0x1E3BD4), create_unit_hook);
	pcreate_unit_hook = h2mod->GetAddress<tcreate_unit_hook>(0x1F32DB, 0x1DE374);

	// Hooks the part of the unit spawn from simulation that handles setting their color data in order to ensure AI do not have their color overridden
	PatchCall(h2mod->GetAddress(0x1F9E34, 0x1E3B9C), set_unit_color_data_hook);
	pset_unit_color_data = h2mod->GetAddress<tset_unit_color_data>(0x6E5C3, 0x6D1BF);
	
}

void H2MOD::ApplyHooks() {
	/* Should store all offsets in a central location and swap the variables based on h2server/halo2.exe*/
	/* We also need added checks to see if someone is the host or not, if they're not they don't need any of this handling. */

	LOG_TRACE_GAME("Applying hooks...");

	/* Labeled "AutoPickup" handler may be proximity to vehicles and such as well */
	PatchCall(h2mod->GetAddress(0x58789, 0x60C81), OnAutoPickUpHandler);

	// hook to initialize stuff before game start
	p_map_cache_load = (map_cache_load)DetourFunc(h2mod->GetAddress<BYTE*>(0x8F62, 0x1F35C), (BYTE*)OnMapLoad, 11);

	// player spawn hook
	p_player_spawn = (player_spawn)DetourFunc(h2mod->GetAddress<BYTE*>(0x55952, 0x5DE4A), (BYTE*)OnPlayerSpawn, 6);

	// game version hook
	p_get_game_version = (get_game_version)DetourFunc(h2mod->GetAddress<BYTE*>(0x1B4BF5, 0x1B0043), (BYTE*)GetGameVersion, 8);

	pplayer_death = (player_death)DetourFunc(h2mod->GetAddress<BYTE*>(0x17B674, 0x152ED4), (BYTE*)OnPlayerDeath, 9);

	pupdate_player_score = (update_player_score)DetourClassFunc(h2mod->GetAddress<BYTE*>(0xD03ED, 0x8C84C), (BYTE*)OnPlayerScore, 12);

	//on_custom_map_change_method = (on_custom_map_change)DetourFunc(h2mod->GetAddress<BYTE*>(0x32176, 0x25738), (BYTE*)onCustomMapChange, 5);



	// disable part of custom map tag verification
	NopFill(GetAddress(0x4FA0A, 0x56C0A), 6);

	// disables profiles/game saves encryption
	PatchWinAPICall(GetAddress(0x9B08A, 0x85F5E), CryptProtectDataHook);
	PatchWinAPICall(GetAddress(0x9AF9E, 0x352538), CryptUnprotectDataHook);
	PatchCall(GetAddress(0x9B09F, 0x85F73), filo_write__encrypted_data_hook);

	ApplyUnitHooks();
	mapManager->applyGamePatches();

	// bellow hooks applied to specific executables
	if (this->Server == false) {
		HMODULE h = GetModuleHandleA("DiscordHook.dll");

		//DWORD random_memory = malloc()
		LOG_TRACE_GAME("Applying client hooks...");
		/* These hooks are only built for the client, don't enable them on the server! */

		PatchCall(h2mod->GetAddress(0x288B5), game_is_minimized_hook);

		p_verify_game_version_on_join = (verify_game_version_on_join)DetourFunc(h2mod->GetAddress<BYTE*>(0x1B4C14), (BYTE*)VerifyGameVersionOnJoin, 5);

		p_verify_executable_version = (verify_executable_version)DetourFunc(h2mod->GetAddress<BYTE*>(0x1B4C32), (BYTE*)VerifyExecutableVersion, 8);

		//pload_wgit = (tload_wgit)DetourClassFunc(h2mod->GetAddress<BYTE*>(0x2106A2), (BYTE*)OnWgitLoad, 13);

		//intercept_map_load_method = (intercept_map_load)DetourClassFunc(h2mod->GetAddress<BYTE*>(0xC259B), (BYTE*)interceptMapLoad, 13);

		show_error_screen_method = (show_error_screen)DetourFunc(h2mod->GetAddress<BYTE*>(0x20E15A), (BYTE*)showErrorScreen, 8);

		//TODO: turn on if you want to debug halo2.exe from start of process
		//is_debugger_present_method = (is_debugger_present)DetourFunc(h2mod->GetAddress<BYTE*>(0x39B394), (BYTE*)isDebuggerPresent, 5);

		//TODO: use for object spawn hooking
		//0x132163
		//object_p_hook_method = (object_p_hook)DetourFunc(h2mod->GetAddress<BYTE*>(0x132163), (BYTE*)objectPHook, 6);

		//TODO: expensive, use for debugging/searching
		//string_display_hook_method = (string_display_hook)DetourFunc(h2mod->GetAddress<BYTE*>(0x287AB5), (BYTE*)stringDisplayHook, 5);

		//pResetRound=(ResetRounds)DetourFunc(h2mod->GetAddress<BYTE*>(0x6B1C8), (BYTE*)OnNextRound, 7);

		/*
		WritePointer(h2mod->GetAddress<BYTE*>(0x1F0B3A), player_add_packet_handler);
		WritePointer(h2mod->GetAddress<BYTE*>(0x1F0B80), player_remove_packet_handler);
		*/

		p_change_local_team = (change_team)DetourFunc(h2mod->GetAddress<BYTE*>(0x2068F2), (BYTE*)changeTeam, 8);

		// hook the print command to redirect the output to our console
		PatchCall(GetAddress(0xE9E50), print_to_console);

		calculate_model_lod = GetAddress(0x19CA3E);
		calculate_model_lod_detour_end = GetAddress(0x19CDA3 + 5);
		WriteJmpTo(GetAddress(0x19CDA3), calculate_model_lod_detour);

		// set max model quality to L6
		WriteValue(GetAddress(0x190B38 + 1), 5);

		pfn_c000bd114 = (tfn_c000bd114)DetourFunc(h2mod->GetAddress<BYTE*>(0xbd114), (BYTE*)fn_c000bd114_IsSkullEnabled, 5);

		PatchCall(GetAddress(0x182d6d), GrenadeChainReactIsEngineMPCheck);
		PatchCall(GetAddress(0x92C05), BansheeBombIsEngineMPCheck);
		PatchCall(GetAddress(0x13ff75), FlashlightIsEngineSPCheck);

		PatchCall(h2mod->GetAddress(0x226702), game_mode_engine_draw_team_indicators);

#pragma region squirrel loading hooks
		pvariant_list_button_handler = (tvariant_list_button_handler)DetourClassFunc(h2mod->GetAddress<BYTE*>(0x251B7E), (BYTE*)variant_list_button_handler, 13); // loads the selected script
		pLoadSaveGame = (tLoadSaveGame)DetourFunc(h2mod->GetAddress<BYTE*>(0x9B0D0), (BYTE*)LoadSaveGameHook, 13); // updates the list to include the script.
		pReadVariantToMemory = (tReadVariantToMemory)DetourFunc(h2mod->GetAddress<BYTE*>(0x46596), (BYTE*)ReadVariantToMemory, 13); // finds all scripts and injects them into memory.
#pragma endregion

#pragma region split-screen fixes
		/* 
			We re-implement the input function that processes key states and actually applies them to the buffer individual bipeds read from,
			This was implemented specifically for COOP but can be used to do other things such as implementing a proper raw-mouse input fix.
			TODO(PermaNull): Implement the rest of the input systems to allow for more than 2 players and remove the current restrictions.
		*/
		//InitializeInputFixes();	

		/*
		Fix video, this should only be turned on when running split-screen,
		TODO(PermaNull): Most likely will keep all the hooks active but call original functions if not in COOP/split-screen.
		*/
		//SplitFixVideoInitialize();
#pragma endregion


		/* 
			An attempt at adding an additional variant/game engine category (E.x. Slayer,KOTH, Infection).
			This will work when Himanshu gets meta injection for creating wigt panels working.
		*/
		//pConVariantCategoryList = (tConVariantCategoryList)DetourClassFunc(h2mod->GetAddress<BYTE*>(0x249D5E), (BYTE*)ConVariantCategoryList, 13);
		//pc_game_engine_category_list_create_labels = (tc_game_engine_category_list_create_labels)DetourClassFunc(h2mod->GetAddress<BYTE*>(0x2497D8), (BYTE*)c_game_engine_category_list_create_labels, 9);
		
		//Hooked to fix custom map images.
		Codecave(GetAddress(0x593F0), load_map_data_for_display,0);
				
		//Initialise_tag_loader();
	}
	else {

		LOG_TRACE_GAME("Applying dedicated server hooks...");
		ServerConsole::ApplyHooks();
	}
}

VOID CALLBACK UpdateDiscordStateTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	update_player_count();
}




void H2MOD::Initialize()
{
	if (!h2mod->Server)
	{
		if (H2Config_raw_input)
			Mouseinput::Initialize();

		if (H2Config_discord_enable && H2GetInstanceId() == 1) {
			// Discord init
			DiscordInterface::SetDetails("Startup");
			DiscordInterface::Init();
			SetTimer(NULL, 0, 5000, UpdateDiscordStateTimer);
		}
	}

	LOG_TRACE_GAME("H2MOD - Initialized v0.5a");
	LOG_TRACE_GAME("H2MOD - BASE ADDR {:x}", this->GetBase());


	ScriptEngine::sqSessionStart();

	h2mod->ApplyHooks();
}

void H2MOD::Deinitialize() {

}
