#include <Windows.h>
#include <Psapi.h>
#include <string>
#include <tchar.h>

#include "Log.hpp"

DWORD CheckProcessNameThread(void* userdata)
{
	TCHAR szFileName[MAX_PATH];

	GetModuleBaseName(GetCurrentProcess(), GetModuleHandle(NULL), szFileName, MAX_PATH);

	if (_tcscmp(szFileName, L"GTA5.exe") != 0)
	{
		FreeLibraryAndExitThread((HMODULE)userdata, 0);

		return 0;
	}

	// Found GTA5.exe, inject main DLL
	LOGSTRF("Found GTA5.exe with shim DLL, injecting main DLL..\n");

	LoadLibrary(L"F:\\moddedgta\\OVRInject.dll");

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&CheckProcessNameThread, (void*)hModule, 0, NULL);

		Sleep(250);
	}

	return 1;
}

