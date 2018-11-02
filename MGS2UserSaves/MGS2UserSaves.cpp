#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502

#include <windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>

#include "Patterns.h"
#include "MemoryMgr.h"

#pragma comment(lib, "shlwapi.lib")

static char* AbsoluteSavePath;
static void SetUpSavesPath_New()
{
	// Relocate saves to Documents/My Games/Metal Gear Solid 2 Substance
	// As a fallback, use stock game behaviour (../savedata)
	CHAR fullPath[MAX_PATH];
	if ( SUCCEEDED(SHGetFolderPathA( nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, fullPath )) )
	{
		PathAppendA( fullPath, "My Games" );
		CreateDirectoryA( fullPath, nullptr );

		PathCombineA( AbsoluteSavePath, fullPath, "METAL GEAR SOLID 2 SUBSTANCE" );
	}
	else
	{
		// Fallback code, same as original game
		GetModuleFileNameA( nullptr, fullPath, sizeof(fullPath)/sizeof(fullPath[0]) );
		PathRemoveFileSpecA( fullPath );

		PathCombineA( AbsoluteSavePath, fullPath, "..\\savedata" );
	}

	CreateDirectoryA( AbsoluteSavePath, nullptr );
	PathAddBackslashA( AbsoluteSavePath );
}

static void InstallHooks()
{
	using namespace hook;
	using namespace Memory::VP;

	// Move saves directory to Documents/My Games/Metal Gear Solid 2
	{
		auto getSavesDir = pattern( "53 56 57 68 ? ? ? ? BF" ).count(1);
		if ( getSavesDir.size() == 1 )
		{
			AbsoluteSavePath = *getSavesDir.get_first<char*>( 0x19 + 1 );
			InjectHook( getSavesDir.get_first( -9 ), SetUpSavesPath_New, PATCH_JUMP );
		}
	}
}


extern "C"
{
	static LONG InitCount = 0;
	__declspec(dllexport) void InitializeASI()
	{
		if ( _InterlockedCompareExchange( &InitCount, 1, 0 ) != 0 ) return;
		InstallHooks();
	}
}