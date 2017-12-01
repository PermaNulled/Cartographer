#include "H2Tweaks.h"
#include "H2Startup.h"
#include "H2Config.h"
#include "GSUtils.h"
#include "Hook.h"
#include "H2OnscreenDebugLog.h"


typedef int(__cdecl *thookServ1)(HKEY, LPCWSTR);
thookServ1 phookServ1;
int __cdecl LoadRegistrySettings(HKEY hKey, LPCWSTR lpSubKey) {
	char result =
		phookServ1(hKey, lpSubKey);
	addDebugText("Post Server Registry Read.");
	if (strlen(H2Config_dedi_server_name) > 0) {
		wchar_t* PreLoadServerName = (wchar_t*)((BYTE*)H2BaseAddr + 0x3B49B4);
		swprintf(PreLoadServerName, 15, L"%hs", H2Config_dedi_server_name);
		//char temp[27];
		//snprintf(temp, 27, "%ws", dedi_server_name);
		//MessageBoxA(NULL, temp, "Server Pre name thingy", MB_OK);
		wchar_t* LanServerName = (wchar_t*)((BYTE*)H2BaseAddr + 0x52042A);
		swprintf(LanServerName, 2, L"");
	}
	if (strlen(H2Config_dedi_server_playlist) > 0) {
		wchar_t* ServerPlaylist = (wchar_t*)((BYTE*)H2BaseAddr + 0x3B3704);
		swprintf(ServerPlaylist, 256, L"%hs", H2Config_dedi_server_playlist);
	}
	return result;
}

typedef int(__stdcall *thookServ2)();
thookServ2 phookServ2;
int __stdcall PreReadyLoad() {
	MessageBoxA(NULL, "aaaaaaas", "PreCrash", MB_OK);
	int result =
		phookServ2();
	MessageBoxA(NULL, "aaaaaaasaaaaaa", "PostCrash", MB_OK);
	if (strlen(H2Config_dedi_server_name) > 0) {
		wchar_t* PreLoadServerName = (wchar_t*)((BYTE*)H2BaseAddr + 0x52042A);
		swprintf(PreLoadServerName, 32, L"%hs", H2Config_dedi_server_name);
		char temp[27];
		snprintf(temp, 32, H2Config_dedi_server_name);
		MessageBoxA(NULL, temp, "AAA Server Pre name thingy", MB_OK);
	}
	return result;
}

static bool NotDisplayIngameChat() {
	int GameGlobals = (int)*(int*)((DWORD)H2BaseAddr + 0x482D3C);
	DWORD* GameEngine = (DWORD*)(GameGlobals + 0x8);
	BYTE* GameState = (BYTE*)((DWORD)H2BaseAddr + 0x420FC4);

	if (H2Config_hide_ingame_chat) {
		int GameTimeGlobals = (int)*(int*)((DWORD)H2BaseAddr + 0x4C06E4);
		DWORD* ChatOpened = (DWORD*)(GameTimeGlobals + 0x354);//MP Only?
		if (*ChatOpened == 2) {
			extern void hotkeyFuncToggleHideIngameChat();
			hotkeyFuncToggleHideIngameChat();
		}
	}

	if (H2Config_hide_ingame_chat) {
		return true;
	}
	else if (*GameEngine != 3 && *GameState == 3) {
		//Enable chat in engine mode and game state mp.
		return false;
	}
	else {
		//original test - if is campaign
		return *GameEngine == 1;
	}
}

typedef char(__cdecl *thookChangePrivacy)(int);
thookChangePrivacy phookChangePrivacy;
char __cdecl HookChangePrivacy(int privacy) {
	char result =
		phookChangePrivacy(privacy);
	if (result == 1 && privacy == 0) {
		pushHostLobby();
	}
	return result;
}


void postConfig() {
	//FIXME
	/*
	addDebugText("Post Config Port & Login Mutex.");
	char H2Config_login_token[33] = { "" };
	wchar_t mutexName[255];
	swprintf(mutexName, L"Halo2Login#%s", H2Config_login_token);
	HANDLE mutex = CreateMutex(0, TRUE, mutexName);
	DWORD lastErr = GetLastError();
	char token_censored[33];
	strncpy(token_censored, H2Config_login_token, 32);
	memset(token_censored + 32, 0, 1);
	memset(token_censored + 4, '*', 24);
	if (lastErr == ERROR_ALREADY_EXISTS) {
		char NotificationPlayerText[120];
		sprintf(NotificationPlayerText, "Player Login Session %s already exists!\nOld session has been invalidated!", token_censored);
		addDebugText(NotificationPlayerText);
		MessageBoxA(NULL, NotificationPlayerText, "LOGIN OVERRIDDEN WARNING!", MB_OK);
	}
	char NotificationText4[120];
	sprintf(NotificationText4, "Login Token: %s.", token_censored);
	addDebugText(NotificationText4);
	*/

	wchar_t mutexName2[255];
	swprintf(mutexName2, L"Halo2BasePort#%d", H2Config_base_port);
	HANDLE mutex2 = CreateMutex(0, TRUE, mutexName2);
	DWORD lastErr2 = GetLastError();
	if (lastErr2 == ERROR_ALREADY_EXISTS) {
		char NotificationPlayerText[120];
		sprintf(NotificationPlayerText, "Base port %d is already bound to!\nExpect MP to not work!", H2Config_base_port);
		addDebugText(NotificationPlayerText);
		MessageBoxA(NULL, NotificationPlayerText, "BASE PORT BIND WARNING!", MB_OK);
	}
	char NotificationText5[120];
	sprintf(NotificationText5, "Base port: %d.", H2Config_base_port);
	addDebugText(NotificationText5);

}


void xtraHac() {
	if (!H2IsDediServer) {
		//Enable Skulls in MP
		BYTE assmEnableMPSkulls[] = { 0x90, 0x90 };
		WriteBytesASM(H2BaseAddr + 0xBD125, assmEnableMPSkulls, 2);

		//Enable Grenade Chain Reactions in MP
		BYTE assmEnableGrenadeChainReact[] = { 0x90, 0x90 };
		WriteBytesASM(H2BaseAddr + 0x182D74, assmEnableGrenadeChainReact, 2);

		//Enable Explosion Physics on the Client Side in MP (grenade jumping etc.)
		BYTE assmEnableExplosionPhysicsMPClient[] = { 0x1E };
		WriteBytesASM(H2BaseAddr + 0x17A44B, assmEnableExplosionPhysicsMPClient, 1);

		//Enable Ranks Patch
		BYTE assmPatchEnableRanks[] = { 0x75, 0x00, 0xC6, 0x00, 0xFF, 0xC6, 0x40, 0xFF, 48 };
		WriteBytesASM(H2BaseAddr + 0x1B2C27, assmPatchEnableRanks, 9);

		//Enables Speed Hacking
		BYTE assmSpeedPatch[8];
		memset(assmSpeedPatch, 0, 8);
		WriteBytesASM(H2BaseAddr + 0x6AB7f, assmSpeedPatch, 8);

		//Enables the Banshee Bomb in MP
		BYTE assmEnableMPBansheeBomb[] = { 0x09 };
		WriteBytesASM(H2BaseAddr + 0x92C06, assmEnableMPBansheeBomb, 1);

		//Disables Vehicle Eject on Flipover (host required)
		BYTE assmDisableVehicleFlipEject[] = { 0xEB };
		WriteBytesASM(H2BaseAddr + 0x159e5d, assmDisableVehicleFlipEject, 1);

		//PatchCall(H2BaseAddr + 0x228579, (DWORD)sub_BD114_blind_1);
		//PatchCall(H2BaseAddr + 0x223955, (DWORD)sub_BD114_blind_2);

		DWORD dwBack;

		//psub_BD114 = (tsub_BD114)DetourFunc((BYTE*)H2BaseAddr + 0xBD114, (BYTE*)sub_BD114, 5);
		//VirtualProtect(psub_BD114, 4, PAGE_EXECUTE_READWRITE, &dwBack);

		//psub_22657B = (tsub_22657B)DetourFunc((BYTE*)H2BaseAddr + 0x22657B, (BYTE*)sub_22657B, 6);
		//VirtualProtect(psub_22657B, 4, PAGE_EXECUTE_READWRITE, &dwBack);

		//psub_225827 = (tsub_225827)DetourFunc((BYTE*)H2BaseAddr + 0x225827, (BYTE*)sub_225827, 11);
		//VirtualProtect(psub_225827, 4, PAGE_EXECUTE_READWRITE, &dwBack);

		//psub_225910 = (tsub_225910)DetourClassFunc((BYTE*)H2BaseAddr + 0x225910, (BYTE*)sub_225910, 11);
		//VirtualProtect(psub_225910, 4, PAGE_EXECUTE_READWRITE, &dwBack);


	}
}



void InitH2Tweaks() {
	postConfig();

	addDebugText("Begin Startup Tweaks.");

	if (H2IsDediServer) {
		DWORD dwBack;

		phookServ1 = (thookServ1)DetourFunc((BYTE*)H2BaseAddr + 0x8EFA, (BYTE*)LoadRegistrySettings, 11);
		VirtualProtect(phookServ1, 4, PAGE_EXECUTE_READWRITE, &dwBack);

		//phookServ2 = (thookServ2)DetourFunc((BYTE*)H2BaseAddr + 0xBA3C, (BYTE*)PreReadyLoad, 11);
		//VirtualProtect(phookServ2, 4, PAGE_EXECUTE_READWRITE, &dwBack);
	}
	else {//is client

		DWORD dwBack;
		//Hook a function which changes the party privacy to detect if the lobby becomes open.
		//Problem is if you want to set it via mem poking, it won't push the lobby to the master automatically.
		//phookChangePrivacy = (thookChangePrivacy)DetourFunc((BYTE*)H2BaseAddr + 0x2153ce, (BYTE*)HookChangePrivacy, 11);
		//VirtualProtect(phookChangePrivacy, 4, PAGE_EXECUTE_READWRITE, &dwBack);

		//Scrapped for now, maybe.
		DWORD tempResX = 0;
		DWORD tempResY = 0;

		HKEY hKeyResolution = NULL;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Halo 2\\Video Settings", 0, KEY_READ, &hKeyResolution) == ERROR_SUCCESS) {
			GetDWORDRegKey(hKeyResolution, L"ScreenResX", &tempResX);
			GetDWORDRegKey(hKeyResolution, L"ScreenResY", &tempResY);
			RegCloseKey(hKeyResolution);
		}

		if (H2Config_custom_resolution_x > 0 && H2Config_custom_resolution_y > 0) {
			if (H2Config_custom_resolution_x != (int)tempResX || H2Config_custom_resolution_y != (int)tempResY) {
				LSTATUS err;
				if (err = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Halo 2\\Video Settings", 0, KEY_ALL_ACCESS, &hKeyResolution) == ERROR_SUCCESS) {
					RegSetValueEx(hKeyResolution, L"ScreenResX", NULL, REG_DWORD, (const BYTE*)&H2Config_custom_resolution_x, sizeof(H2Config_custom_resolution_x));
					RegSetValueEx(hKeyResolution, L"ScreenResY", NULL, REG_DWORD, (const BYTE*)&H2Config_custom_resolution_y, sizeof(H2Config_custom_resolution_y));
					RegCloseKey(hKeyResolution);
				}
				else {
					char errorMsg[200];
					sprintf(errorMsg, "Error: 0x%x. Unable to make Screen Resolution changes.\nPlease try restarting Halo 2 with Administrator Privileges.", err);
					addDebugText(errorMsg);
					MessageBoxA(NULL, errorMsg, "Registry Write Error", MB_OK);
					exit(EXIT_FAILURE);
				}
			}
		}

		bool IntroHQ = true;//clients should set on halo2.exe -highquality

		if (H2Config_skip_intro) {
			BYTE assmIntroSkip[] = { 0x3F };
			WriteBytesASM(H2BaseAddr + 0x221C0E, assmIntroSkip, 1);
		}

		if (!H2Config_skip_intro && IntroHQ) {
			BYTE assmIntroHQ[] = { 0xEB };
			WriteBytesASM(H2BaseAddr + 0x221C29, assmIntroHQ, 1);
		}

		//Allows unlimited clients
		BYTE assmUnlimitedClients[41];
		memset(assmUnlimitedClients, 0x00, 41);
		WriteBytesASM(H2BaseAddr + 0x39BCF0, assmUnlimitedClients, 41);

		//Allows on a remote desktop connection
		BYTE assmRemoteDesktop[] = { 0xEB };
		WriteBytesASM(H2BaseAddr + 0x7E54, assmRemoteDesktop, 1);

		//multi-process splitscreen input hacks
		if (H2Config_disable_ingame_keyboard) {
			//Allows to repeat last movement when lose focus in mp, unlocks METHOD E from point after intro vid
			BYTE getFocusB[] = { 0x00 };
			WriteBytesASM(H2BaseAddr + 0x2E3C5, getFocusB, 1);
			//Allows input when not in focus.
			BYTE getFocusE[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			WriteBytesASM(H2BaseAddr + 0x2F9EA, getFocusE, 6);
			WriteBytesASM(H2BaseAddr + 0x2F9FC, getFocusE, 6);
			WriteBytesASM(H2BaseAddr + 0x2FA09, getFocusE, 6);
			//Disables the keyboard only when in-game and not in a menu.
			BYTE disableKeyboard1[] = { 0x90, 0x90, 0x90 };
			WriteBytesASM(H2BaseAddr + 0x2FA8A, disableKeyboard1, 3);
			BYTE disableKeyboard2[] = { 0x00 };
			WriteBytesASM(H2BaseAddr + 0x2FA92, disableKeyboard2, 1);
			BYTE disableKeyboard3[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			WriteBytesASM(H2BaseAddr + 0x2FA67, disableKeyboard3, 6);
		}

		//Disables the ESRB warning (only occurs for English Language).
		//disables the one if no intro vid occurs.
		BYTE& ESRB = *((BYTE*)H2BaseAddr + 0x411030);
		ESRB = 0;
		//disables the one after the intro video.
		BYTE assmIntroESRBSkip[] = { 0x00 };
		WriteBytesASM(H2BaseAddr + 0x3a0fa, assmIntroESRBSkip, 1);
		WriteBytesASM(H2BaseAddr + 0x3a1ce, assmIntroESRBSkip, 1);

		//Redirects the is_campaign call that the in-game chat renderer makes so we can show/hide it as we like.
		PatchCall(H2BaseAddr + 0x22667B, (DWORD)NotDisplayIngameChat);
		PatchCall(H2BaseAddr + 0x226628, (DWORD)NotDisplayIngameChat);
	}
	addDebugText("End Startup Tweaks.");

	xtraHac();

}

void DeinitH2Tweaks() {

}
