#pragma once
#include "stdafx.h"

void InitH2Startup();
void InitH2Startup2();
void DeinitH2Startup();
int H2GetInstanceId();

enum H2Types
{
	H2Game,
	H2Server,
	Invalid
};

class ProcessInfo
{

public:
	HMODULE base;
	H2Types process_type = H2Types::Invalid;
};

extern ProcessInfo game_info;

extern bool H2IsDediServer;
extern bool H2DediIsLiveMode;
extern DWORD H2BaseAddr;
extern wchar_t* H2ProcessFilePath;
extern wchar_t* H2AppDataLocal;
extern wchar_t* FlagFilePathConfig;
extern HWND H2hWnd;
