#pragma once

#include <dxgi.h>
#include <d3d11.h>

#include "MinHook.h"
#include "Vive/HMDSupport.hpp"

extern HMODULE dxgiModule;

namespace OVRInject {
	typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY)(
		REFIID				riid,
		_Out_				void				**pFactory
	);

	bool first_deviceAndChain = true;

	void* pCreateDXGIFactory1 = nullptr;
	void* Original_CreateDXGIFactory;

	HRESULT Proxy_CreateDXGIFactory(
		REFIID				riid,
		_Out_				void				**pFactory
	)
	{
		LOGWNDF("Called CreateDXGIFactory : Original 0x%p\n", Original_CreateDXGIFactory);

		if (pCreateDXGIFactory1 == nullptr)
		{
			pCreateDXGIFactory1 = GetProcAddress(dxgiModule, "CreateDXGIFactory1");
			LOGWNDF("DXGIFACTORTY1: 0x%p\n", pCreateDXGIFactory1);
		}

		IDXGIFactory1 *nFactory;

		HRESULT result = ((PFN_CREATE_DXGI_FACTORY)pCreateDXGIFactory1)(
			__uuidof(IDXGIFactory1),
			(void**) &nFactory
			);

		LOGWNDF("Created nFactory 0x%p\n", *nFactory);

		nFactory->QueryInterface(__uuidof(IDXGIFactory), pFactory);

		LOGWNDF("Got oldass interface pFactory 0x%p\n", *pFactory);

		return result;
	};

	struct Detour_CreateDXGIFactory : public Detour
	{
		Detour_CreateDXGIFactory(void* from) : Detour(from)
		{
			target = Proxy_CreateDXGIFactory;
			original = &Original_CreateDXGIFactory;
		};
	};

	void* Original_D3D11CreateDevice;

	HRESULT Proxy_D3D11CreateDevice(
		_In_opt_			IDXGIAdapter        *pAdapter,
		D3D_DRIVER_TYPE     DriverType,
		HMODULE             Software,
		UINT                Flags,
		_In_opt_	const	D3D_FEATURE_LEVEL   *pFeatureLevels,
		UINT                FeatureLevels,
		UINT                SDKVersion,
		_Out_opt_			ID3D11Device        **ppDevice,
		_Out_opt_			D3D_FEATURE_LEVEL   *pFeatureLevel,
		_Out_opt_			ID3D11DeviceContext **ppImmediateContext
		)
	{
		LOGSTRF("Called D3D11CreateDevice : Original 0x%p\n", Original_D3D11CreateDevice);

		return ((PFN_D3D11_CREATE_DEVICE)Original_D3D11CreateDevice)(
			pAdapter,
			DriverType,
			Software,
			Flags,
			pFeatureLevels,
			FeatureLevels,
			SDKVersion,
			ppDevice,
			pFeatureLevel,
			ppImmediateContext
			);
	};

	struct Detour_D3D11CreateDevice : public Detour
	{
		Detour_D3D11CreateDevice(void* from) : Detour(from)
		{
			target = Proxy_D3D11CreateDevice;
			original = &Original_D3D11CreateDevice;
		};
	};

	HMDSupport hmdSupport;

	typedef HRESULT(__stdcall *PresentHook)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	PresentHook Original_PresentHook = 0;

	HRESULT __stdcall hookedPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if (first_deviceAndChain)
		{
			LOGSTRF("Initializing HMDSupport..\n");

			ID3D11Device* device;
			pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);

			hmdSupport.Initialize(pSwapChain, device, false);

			first_deviceAndChain = false;

			return S_OK;
		}

		// Steal the backbuffer from the D3D rendering context

		ID3D11Texture2D* pBuffer;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);

		{
			hmdSupport.SubmitTexture(0, pBuffer);
			hmdSupport.PostPresent();
		}

		pBuffer->Release();

		return S_OK;
	}

	typedef HRESULT(__stdcall *CreateTexture2DHook)(
		_In_		const	D3D11_TEXTURE2D_DESC		*pDesc,
		_In_opt_	const	D3D11_SUBRESOURCE_DATA	*pInitialData,
		_Out_opt_			ID3D11Texture2D			**ppTextureData
		);
	CreateTexture2DHook Original_CreateTexture2DHook = 0;

	HRESULT __stdcall hookedCreateTexture2D(
		_In_		const	D3D11_TEXTURE2D_DESC		*pDesc,
		_In_opt_	const	D3D11_SUBRESOURCE_DATA	*pInitialData,
		_Out_opt_			ID3D11Texture2D			**ppTextureData
	)
	{
		return Original_CreateTexture2DHook(pDesc, pInitialData, ppTextureData);
	}

	void* Original_D3D11CreateDeviceAndSwapChain;

	// DetourFuncVTable*
	const void* DetourFuncVTable(SIZE_T* src, const BYTE* dest, const DWORD index)
	{
		DWORD dwVirtualProtectBackup;

		SIZE_T* const indexPtr = &src[index];
		const void* origFunc = (void*)*indexPtr;

		VirtualProtect(indexPtr, sizeof(SIZE_T), PAGE_EXECUTE_READWRITE, &dwVirtualProtectBackup);
		*indexPtr = (SIZE_T)dest;
		VirtualProtect(indexPtr, sizeof(SIZE_T), dwVirtualProtectBackup, &dwVirtualProtectBackup);

		return origFunc;
	}

	HRESULT Proxy_D3D11CreateDeviceAndSwapChain(
		_In_opt_			IDXGIAdapter         *pAdapter,
		D3D_DRIVER_TYPE     DriverType,
		HMODULE             Software,
		UINT                Flags,
		_In_opt_	const	D3D_FEATURE_LEVEL    *pFeatureLevels,
		UINT                FeatureLevels,
		UINT                SDKVersion,
		_In_opt_	const	DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
		_Out_opt_			IDXGISwapChain       **ppSwapChain,
		_Out_opt_			ID3D11Device         **ppDevice,
		_Out_opt_			D3D_FEATURE_LEVEL    *pFeatureLevel,
		_Out_opt_			ID3D11DeviceContext  **ppImmediateContext
		)
	{
		LOGSTRF("Called D3D11CreateDeviceAndSwapChain : Original 0x%p\n", Original_D3D11CreateDeviceAndSwapChain);
		
		HRESULT result = ((PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)Original_D3D11CreateDeviceAndSwapChain)(
			pAdapter,
			DriverType,
			Software,
			Flags,
			pFeatureLevels,
			FeatureLevels,
			SDKVersion,
			pSwapChainDesc,
			ppSwapChain,
			ppDevice,
			pFeatureLevel,
			ppImmediateContext
			);

		/*
		if (ppDevice != 0)
		{
			ID3D11Device* pDevice = *ppDevice;

			if (pDevice == 0)
			{
				return result;
			}

			DWORD64* vtable = (DWORD64*)pDevice;
			vtable = (DWORD64*)vtable[0];

			Original_CreateTexture2DHook = (CreateTexture2DHook)DetourFuncVTable(vtable, (BYTE*)hookedCreateTexture2D, 8);
		}
		*/

		if (ppSwapChain != 0)
		{
			IDXGISwapChain* pSwapChain = *ppSwapChain;

			if (pSwapChain == 0)
			{
				return result;
			}

			DWORD64* vtable = (DWORD64*)pSwapChain;
			vtable = (DWORD64*)vtable[0];

			Original_PresentHook = (PresentHook)DetourFuncVTable(vtable, (BYTE*)hookedPresent, 8);
		}

		return result;
	};

	struct Detour_D3D11CreateDeviceAndSwapChain : public Detour
	{
		Detour_D3D11CreateDeviceAndSwapChain(void* from) : Detour(from)
		{
			target = Proxy_D3D11CreateDeviceAndSwapChain;
			original = &Original_D3D11CreateDeviceAndSwapChain;
		};
	};
}