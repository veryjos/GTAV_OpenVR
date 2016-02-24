#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>

#include "Detour/D3D11DetourManager.hpp"

#include "Log.hpp"

using namespace OVRInject;

DetourManager* detourManager;

void init();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		LOGSTRF("Attached main DLL to GTA: V\n");

		init();

		break;
	}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void init()
{
	detourManager = new D3D11DetourManager();
}