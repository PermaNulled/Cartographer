#pragma once

#include "Blam\Maths\Maths.h"
#include "Blam\Cache\DataTypes.h"

enum ObjectTeam : BYTE
{
	// MP
	Red = 0,
	Blue = 1,
	Yellow = 2,
	Green = 3,
	Purple = 4,
	Orange = 5,
	Bronwn = 6,
	Pink = 7,
	// SP
	Default = 0,
	player = 1,
	Human = 2,
	Covenant = 3,
	Flood = 4,
	Sentinel = 5,
	Heretic = 6,
	Prophet = 7,

	// unasigned team ids
	Unused8 = 8,
	Unused9 = 9,
	Unused10 = 10,
	Unused11 = 11,
	Unused12 = 12,
	Unused13 = 13,
	Unused14 = 14,
	Unused15 = 15,

	// Shared
	None = 255
};

enum class ObjectType : signed char
{
	/* Because these are unsigned, they can only be positive.*/
	_object_type_object = -4,
	_object_type_device = -3,
	_object_type_item = -2,
	_object_type_unit = -1,

	biped = 0,
	vehicle,
	weapon,
	equipment,
	garbage,
	projectile,
	scenery,
	machine,
	control,
	light_fixture,
	sound_scenery,
	crate,
	creature,
};

enum class UnitWeapons
{
	PrimaryWeapon,
	SecondaryWeapon,
	DualWeildWeapon
};

enum class Grenades : BYTE
{
	Fragmentation,
	Plasma
};

enum class WeaponIndex : WORD
{
	Primary = 0xFF00,
	Secondary = 0xFF01,
	DualWeild = 0x0201
};

enum class BipedState : BYTE
{
	mode_ground = 1,
	mode_flying,
	mode_dead,
	mode_sentinel,
	mode_sentinel_2,
	mode_melee
};

//size : depends on the object, there are object definitions for bipeds, projectiles etc...
// for example, the size of the biped object definition is 1152 bytes and for the projectiles is 428 bytes
struct BipedObjectDefinition//To Do
{
	datum TagDefinitionIndex;//0
	DWORD ObjectFlags;//4
	DWORD unk_0;//8
	datum NextIndex;//0xC
	datum CurrentWeaponDatum;//0x10
	datum ParentIndex;//0x14
	WORD UnitInVehicleFlag;//0x18
	INT16 PlacementIndex;//0x1A
	DWORD unk_2[3];//0x1C
	DWORD Location[2];//0x28
	real_point3d Center;//0x30
	FLOAT Radius;//0x3C
	DWORD unk_3[9];//0x40
	real_point3d Placement;//0x64
	real_vector3d Orientation;//0x70
	real_vector3d Up;//0x7C
	real_point3d TranslationalVelocity;//0x88
	real_vector3d AngularVelocity;//0x94
	FLOAT Scale;//0xA0
	BYTE unk_4[6];//0xA4
	ObjectType ObjectType;//0xAA;
	BYTE unk;//0xAB
	INT16 NameListIndex;//0xAC
	BYTE unk_5;//0xAE
	BYTE NetgameEquipmentIndex;//0xAF
	DWORD unk_6;//0xB0
	datum HavokComponentDatum;//0xB4
	DWORD unk_7[11];//0xB8
	FLOAT BodyMaxVitality;//0xE4
	FLOAT ShieldMaxVitality;//0xE8
	FLOAT BodyCurrentVitality;//0xEC
	FLOAT ShieldCurrentVitality;//0xF0
	DWORD ShieldEffects;//0xF4
	DWORD ShieldEffects2;//0xF8
	DWORD ShieldEffects3;//0xFC
	DWORD unk_8;//0x100
	FLOAT ShieldsRechargeTimer;//0x104
	WORD ShieldStun2;//0x108
	BYTE CollisionFlags;//0x10A - flags in general not just collision, it's how the player is killed for instance.
	BYTE HealthFlags;//0x10B
	DWORD unk_9[3];//0x10C
	WORD UnkFlags;//0x118
	WORD UnkFlags2;//0x11A
	BYTE unk_10[14];//0x11C
	WORD AnimationUnk;//0x12A
	BYTE ObjectsAttach;//0x12C
	BYTE unk_11[3];//0x12D
	datum ActorDatum; // 0x130
	BYTE unk_17[4]; //0x138
	DWORD Flags;//0x138
	ObjectTeam Team;//0x13C
	WORD unk_12;//0x13D
	datum PlayerDatum;//0x140
	BYTE unk_13[9];//0x144
	WORD CrouchJumpRelated;//0x150
	FLOAT UnitShoot;//0x152
	BYTE unk_14[102];//0x156
	FLOAT forward_movement_speed;//0x1BC
	FLOAT left_movement_speed;//0x1C0
	BYTE unk_15[16];//0x1C4
	datum TargetObject;//0x1D4
	BYTE unk_16[82];//0x1D8
	WeaponIndex UnitSwitchWeapon;//0x22A
	datum PrimaryWeapon; // 0x22C
	datum SecondaryWeapon; // 0x230
	datum DualWieldWeapon; // 0x234
	BYTE pad[0x18]; //  0x238 

	BYTE CurrentGrenadesIndex; //0x250
	BYTE CurrentGrenadesIndex2; //0x251
	BYTE Frag_Grenades; //0x252
	BYTE Plasma_Grenades; //0x253

	FLOAT ActiveCamoFlagePower;//0x2C4
	FLOAT ActiveCamoFlageTimer;//0x2C8
	FLOAT ActiveCamoFlageDepletionPower;//0x2CC

	BipedState unitState;//0x3F4
	BYTE unk_18[0x21C];
};
static_assert(sizeof(BipedObjectDefinition) == 0x480, "Invalid BipedObjectDefinition size");

struct WeaponObjectDefinition
{
	datum TagDefinitionIndex;//0
	char unk[0x258];
};
static_assert(sizeof(WeaponObjectDefinition) == 0x25C, "Invalid WeaponObjectDefinition size");

struct ObjectHeader {
	__int16 datum_salt; //0x00
	BYTE flags; // 0x02
	ObjectType type; // 0x03
	__int16 unk__;  // 0x04
	__int16 unk_size;  //0x06
	char* object; //0x08 - 
};
static_assert(sizeof(ObjectHeader) == 0xC, "Invalid GameStateObjectHeader size");
