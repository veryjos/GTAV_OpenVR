#include "targetver.h"

#include "D3D11DetourManager.hpp"

#include "D3DHook/D3DHooks.hpp"

#include <d3d11.h>
#include <dxgi.h>

#include "Log.hpp"

using namespace OVRInject;

HMODULE d3d11Module;
HMODULE dxgiModule;

D3D11DetourManager::D3D11DetourManager()
{
	LOGSTRF("Attemping to hook D3D/DXGI device creation methods..\n");

	d3d11Module = LoadLibrary(L"d3d11.dll");
	LOGSTRF("D3D11Module: 0x%p\n", d3d11Module);

	dxgiModule = LoadLibrary(L"dxgi.dll");
	LOGSTRF("DXGIModule: 0x%p\n", dxgiModule);

	MakeDetour<Detour_D3D11CreateDevice>(
		GetProcAddress(d3d11Module, "D3D11CreateDevice")
	);

	MakeDetour<Detour_D3D11CreateDeviceAndSwapChain>(
		GetProcAddress(d3d11Module, "D3D11CreateDeviceAndSwapChain")
	);

	MakeDetour<Detour_CreateDXGIFactory>(
		GetProcAddress(dxgiModule, "CreateDXGIFactory")
	);
}

D3D11DetourManager::~D3D11DetourManager()
{

}