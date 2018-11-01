#include "stdafx.h"
#include <string>
#include <unordered_set>
#include <codecvt>

#include "Util\Hooks\Hook.h"

#include "H2MOD\Modules\UI\UI.h"
#include "H2MOD\Modules\OnScreenDebug\OnScreenDebug.h"
#include "H2MOD\Modules\Startup\Startup.h"


// Fixes pulse resetting the radar
void UI::Patch::RadarPulse() {

	if (H2IsDediServer)
		return;

	DWORD& MapHeaderType = *(DWORD*)(H2BaseAddr + 0x47CD68 + 0x14C);

	if (MapHeaderType != 1)
		return;

	DWORD SharedMapMetaDataPointer = *(DWORD*)(H2BaseAddr + 0x47CD64);

	//Tag : ui\hud\bitmaps\hud_sweeper.bitm
	const DWORD BitmapPropertyOffset = 0x00D93BA8;		//Property : Bitmap
	const BYTE BitmapDefinitionOffset = 0x0C;			//Definition : Format Offset
	const WORD BitmapFormatTypeValue = 0x000A;			//Definition : Format Enum

	WriteValue<WORD>(SharedMapMetaDataPointer + BitmapPropertyOffset + BitmapDefinitionOffset, BitmapFormatTypeValue);
	addDebugText("Radar patched successfully.");
}