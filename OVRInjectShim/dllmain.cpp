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
  LOGWNDF("Found GTA5.exe with shim DLL, injecting main DLL..\n"); 
	LoadLibrary(L"E:\\Games\\Steam\\steamapps\\common\\Grand Theft Auto V\\OVRInject.dll");

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

/*
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
LOGWNDF("Found GTA5.exe with shim DLL, injecting main DLL..\n");

HKEY key;

if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
0, KEY_SET_VALUE | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS)
{
LOGWNDF("Setting up DLL hook for GTA: V..\n");

TCHAR cCurrentPath[FILENAME_MAX * 4];

if (RegGetValue(key, L"AppInit_DLLS", 0, 0, 0, &cCurrentPath[0], 0) != ERROR_SUCCESS) {
LOGWNDF("Couldn't read AppInit_DLLS key\n");

return 0;
}

std::wstring dllPath(cCurrentPath);
dllPath.erase(dllPath.end() - std::wstring(L"\\OVRInjectShim.dll").length(), dllPath.end());
dllPath += L"\\OVRInject.dll";

LOGWNDF("Found DLLPath at %ls\n", dllPath.c_str());

LoadLibrary(dllPath.c_str());
}
else {
LOGWNDF("Failed to open registry key.\n");
return 0;
}

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

*/