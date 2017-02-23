#pragma once
#include "targetver.h"


#include <dxgi.h>
#include <d3d11.h>

#include "MinHook.h"
#include "Vive/HMDSupport.hpp"
#include "Vive/HMDRenderer.hpp"

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
		LOGSTRF("Called CreateDXGIFactory : Original 0x%p\n", Original_CreateDXGIFactory);

		if (pCreateDXGIFactory1 == nullptr)
		{
			pCreateDXGIFactory1 = GetProcAddress(dxgiModule, "CreateDXGIFactory1");
			LOGSTRF("DXGIFACTORTY1: 0x%p\n", pCreateDXGIFactory1);
		}

		IDXGIFactory1 *nFactory;

		HRESULT result = ((PFN_CREATE_DXGI_FACTORY)pCreateDXGIFactory1)(
			__uuidof(IDXGIFactory1),
			(void**) &nFactory
			);

		LOGSTRF("Created nFactory 0x%p\n", *nFactory);

		nFactory->QueryInterface(__uuidof(IDXGIFactory), pFactory);

		LOGSTRF("Got oldass interface pFactory 0x%p, upgrading\n", *pFactory);

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

	HMDSupport* hmdSupport = HMDSupport::Singleton();

	typedef HRESULT(__stdcall *PresentHook)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	PresentHook Original_PresentHook = 0;

  bool firstUnmap = false;

	HRESULT __stdcall hookedPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if (first_deviceAndChain)
		{
			LOGSTRF("Initializing HMDSupport..\n");

			ID3D11Device* device;
			pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);

			hmdSupport->Initialize(pSwapChain, device);

			first_deviceAndChain = false;

			return S_OK;
		}

		// Steal the backbuffer from the D3D rendering context
		ID3D11Texture2D* pBuffer;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);

    // It's okay to use clock here, we're just comparing relative timestamps
		hmdSupport->SubmitFrameTexture(0, pBuffer, clock());

    if (hmdSupport->GetDesktopMirroring())
		  HRESULT result = Original_PresentHook(pSwapChain, SyncInterval, Flags);

		pBuffer->Release();

    firstUnmap = true;

		return 0;
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

  typedef void(__stdcall *UnmapHook)(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
  UnmapHook Original_UnmapHook = 0;

  void __stdcall hookedUnmap(ID3D11DeviceContext* ctx, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
  {
    // This is the first draw since present
    if (firstUnmap) {
      hmdSupport->GetRenderer()->WaitGetPoses();

      firstUnmap = false;
    }

    //LOGSTRF("MEME ME BABY\n");

    //ctx->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
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

  const void* __cdecl DetourFunc(BYTE* src, const BYTE* dest, const DWORD length)
  {
    BYTE* jump = new BYTE[length + 5];

    DWORD dwVirtualProtectBackup;
    VirtualProtect(src, length, PAGE_READWRITE, &dwVirtualProtectBackup);

    memcpy(jump, src, length);
    jump += length;

    jump[0] = 0xE9;
    *(DWORD*)(jump + 1) = (DWORD)(src + length - jump) - 5;

    src[0] = 0xE9;
    *(DWORD*)(src + 1) = (DWORD)(dest - src) - 5;

    VirtualProtect(src, length, dwVirtualProtectBackup, &dwVirtualProtectBackup);

    return jump - length;
  }

  template <class T, typename F>
  int VTableIndex(F f)
  {
    struct VTableCounter
    {
      virtual int Get1() { return 1; }
      virtual int Get2() { return 2; };
      virtual int Get3() { return 3; };
      virtual int Get4() { return 4; };
      virtual int Get5() { return 5; };
      virtual int Get6() { return 6; };
      virtual int Get7() { return 7; };
      virtual int Get8() { return 8; };
      virtual int Get9() { return 9; };
      virtual int Get10() { return 10; };
      virtual int Get11() { return 11; };
      virtual int Get12() { return 12; };
      virtual int Get13() { return 13; };
      virtual int Get14() { return 14; };
      virtual int Get15() { return 15; };
      virtual int Get16() { return 16; };
      virtual int Get17() { return 17; };
      virtual int Get18() { return 18; };
      virtual int Get19() { return 19; };
      virtual int Get20() { return 20; };
      virtual int Get21() { return 21; };
      virtual int Get22() { return 22; };
      virtual int Get23() { return 23; };
      virtual int Get24() { return 24; };
      virtual int Get25() { return 25; };
      virtual int Get26() { return 26; };
      virtual int Get27() { return 27; };
      virtual int Get28() { return 28; };
      virtual int Get29() { return 29; };
      virtual int Get30() { return 30; };
      virtual int Get31() { return 31; };
      virtual int Get32() { return 32; };
      virtual int Get33() { return 33; };
      virtual int Get34() { return 34; };
      virtual int Get35() { return 35; };
      virtual int Get36() { return 36; };
      virtual int Get37() { return 37; };
      virtual int Get38() { return 38; };
      virtual int Get39() { return 39; };
      virtual int Get40() { return 40; };
      virtual int Get41() { return 41; };
      virtual int Get42() { return 42; };
      virtual int Get43() { return 43; };
      virtual int Get44() { return 44; };
      virtual int Get45() { return 45; };
      virtual int Get46() { return 46; };
      virtual int Get47() { return 47; };
      virtual int Get48() { return 48; };
      virtual int Get49() { return 49; };
      virtual int Get50() { return 50; };
      virtual int Get51() { return 51; };
      virtual int Get52() { return 52; };
      virtual int Get53() { return 53; };
      virtual int Get54() { return 54; };
      virtual int Get55() { return 55; };
      virtual int Get56() { return 56; };
      virtual int Get57() { return 57; };
      virtual int Get58() { return 58; };
      virtual int Get59() { return 59; };
      virtual int Get60() { return 60; };
      virtual int Get61() { return 61; };
      virtual int Get62() { return 62; };
      virtual int Get63() { return 63; };
      virtual int Get64() { return 64; };
      virtual int Get65() { return 65; };
      virtual int Get66() { return 66; };
      virtual int Get67() { return 67; };
      virtual int Get68() { return 68; };
      virtual int Get69() { return 69; };
      virtual int Get70() { return 70; };
      virtual int Get71() { return 71; };
      virtual int Get72() { return 72; };
      virtual int Get73() { return 73; };
      virtual int Get74() { return 74; };
      virtual int Get75() { return 75; };
      virtual int Get76() { return 76; };
      virtual int Get77() { return 77; };
      virtual int Get78() { return 78; };
      virtual int Get79() { return 79; };
      virtual int Get80() { return 80; };
      virtual int Get81() { return 81; };
      virtual int Get82() { return 82; };
      virtual int Get83() { return 83; };
      virtual int Get84() { return 84; };
      virtual int Get85() { return 85; };
      virtual int Get86() { return 86; };
      virtual int Get87() { return 87; };
      virtual int Get88() { return 88; };
      virtual int Get89() { return 89; };
      virtual int Get90() { return 90; };
      virtual int Get91() { return 91; };
      virtual int Get92() { return 92; };
      virtual int Get93() { return 93; };
      virtual int Get94() { return 94; };
      virtual int Get95() { return 95; };
      virtual int Get96() { return 96; };
      virtual int Get97() { return 97; };
      virtual int Get98() { return 98; };
      virtual int Get99() { return 99; };
      virtual int Get100() { return 100; };
      virtual int Get101() { return 101; };
      virtual int Get102() { return 102; };
      virtual int Get103() { return 103; };
      virtual int Get104() { return 104; };
      virtual int Get105() { return 105; };
      virtual int Get106() { return 106; };
      virtual int Get107() { return 107; };
      virtual int Get108() { return 108; };
      virtual int Get109() { return 109; };
      virtual int Get110() { return 110; };
      virtual int Get111() { return 111; };
      virtual int Get112() { return 112; };
      virtual int Get113() { return 113; };
      virtual int Get114() { return 114; };
      virtual int Get115() { return 115; };
      virtual int Get116() { return 116; };
      virtual int Get117() { return 117; };
      virtual int Get118() { return 118; };
      virtual int Get119() { return 119; };
      virtual int Get120() { return 120; };
      virtual int Get121() { return 121; };
      virtual int Get122() { return 122; };
      virtual int Get123() { return 123; };
      virtual int Get124() { return 124; };
      virtual int Get125() { return 125; };
      virtual int Get126() { return 126; };
      virtual int Get127() { return 127; };
      virtual int Get128() { return 128; };
      virtual int Get129() { return 129; };
      virtual int Get130() { return 130; };
      virtual int Get131() { return 131; };
      virtual int Get132() { return 132; };
      virtual int Get133() { return 133; };
      virtual int Get134() { return 134; };
      virtual int Get135() { return 135; };
      virtual int Get136() { return 136; };
      virtual int Get137() { return 137; };
      virtual int Get138() { return 138; };
      virtual int Get139() { return 139; };
      virtual int Get140() { return 140; };
      virtual int Get141() { return 141; };
      virtual int Get142() { return 142; };
      virtual int Get143() { return 143; };
      virtual int Get144() { return 144; };
      virtual int Get145() { return 145; };
      virtual int Get146() { return 146; };
      virtual int Get147() { return 147; };
      virtual int Get148() { return 148; };
      virtual int Get149() { return 149; };
      virtual int Get150() { return 150; };
      virtual int Get151() { return 151; };
      virtual int Get152() { return 152; };
      virtual int Get153() { return 153; };
      virtual int Get154() { return 154; };
      virtual int Get155() { return 155; };
      virtual int Get156() { return 156; };
      virtual int Get157() { return 157; };
      virtual int Get158() { return 158; };
      virtual int Get159() { return 159; };
      virtual int Get160() { return 160; };
      virtual int Get161() { return 161; };
      virtual int Get162() { return 162; };
      virtual int Get163() { return 163; };
      virtual int Get164() { return 164; };
      virtual int Get165() { return 165; };
      virtual int Get166() { return 166; };
      virtual int Get167() { return 167; };
      virtual int Get168() { return 168; };
      virtual int Get169() { return 169; };
      virtual int Get170() { return 170; };
      virtual int Get171() { return 171; };
      virtual int Get172() { return 172; };
      virtual int Get173() { return 173; };
      virtual int Get174() { return 174; };
      virtual int Get175() { return 175; };
      virtual int Get176() { return 176; };
      virtual int Get177() { return 177; };
      virtual int Get178() { return 178; };
      virtual int Get179() { return 179; };
      virtual int Get180() { return 180; };
      virtual int Get181() { return 181; };
      virtual int Get182() { return 182; };
      virtual int Get183() { return 183; };
      virtual int Get184() { return 184; };
      virtual int Get185() { return 185; };
      virtual int Get186() { return 186; };
      virtual int Get187() { return 187; };
      virtual int Get188() { return 188; };
      virtual int Get189() { return 189; };
      virtual int Get190() { return 190; };
      virtual int Get191() { return 191; };
      virtual int Get192() { return 192; };
      virtual int Get193() { return 193; };
      virtual int Get194() { return 194; };
      virtual int Get195() { return 195; };
      virtual int Get196() { return 196; };
      virtual int Get197() { return 197; };
      virtual int Get198() { return 198; };
      virtual int Get199() { return 199; };
      virtual int Get200() { return 200; };
      virtual int Get201() { return 201; };
      virtual int Get202() { return 202; };
      virtual int Get203() { return 203; };
      virtual int Get204() { return 204; };
      virtual int Get205() { return 205; };
      virtual int Get206() { return 206; };
      virtual int Get207() { return 207; };
      virtual int Get208() { return 208; };
      virtual int Get209() { return 209; };
      virtual int Get210() { return 210; };
      virtual int Get211() { return 211; };
      virtual int Get212() { return 212; };
      virtual int Get213() { return 213; };
      virtual int Get214() { return 214; };
      virtual int Get215() { return 215; };
      virtual int Get216() { return 216; };
      virtual int Get217() { return 217; };
      virtual int Get218() { return 218; };
      virtual int Get219() { return 219; };
      virtual int Get220() { return 220; };
      virtual int Get221() { return 221; };
      virtual int Get222() { return 222; };
      virtual int Get223() { return 223; };
      virtual int Get224() { return 224; };
      virtual int Get225() { return 225; };
      virtual int Get226() { return 226; };
      virtual int Get227() { return 227; };
      virtual int Get228() { return 228; };
      virtual int Get229() { return 229; };
      virtual int Get230() { return 230; };
      virtual int Get231() { return 231; };
      virtual int Get232() { return 232; };
      virtual int Get233() { return 233; };
      virtual int Get234() { return 234; };
      virtual int Get235() { return 235; };
      virtual int Get236() { return 236; };
      virtual int Get237() { return 237; };
      virtual int Get238() { return 238; };
      virtual int Get239() { return 239; };
      virtual int Get240() { return 240; };
      virtual int Get241() { return 241; };
      virtual int Get242() { return 242; };
      virtual int Get243() { return 243; };
      virtual int Get244() { return 244; };
      virtual int Get245() { return 245; };
      virtual int Get246() { return 246; };
      virtual int Get247() { return 247; };
      virtual int Get248() { return 248; };
      virtual int Get249() { return 249; };
      virtual int Get250() { return 250; };
      virtual int Get251() { return 251; };
      virtual int Get252() { return 252; };
      virtual int Get253() { return 253; };
      virtual int Get254() { return 254; };
      virtual int Get255() { return 255; };
      // ... more ...
    } vt;

    T* t = reinterpret_cast<T*>(&vt);

    typedef int (T::*GetIndex)();
    GetIndex getIndex = (GetIndex)f;
    return (t->*getIndex)();
  }
  
  template <class T>
  int SeeBits(T func)
  {
    union
    {
      T ptr;
      int i;
    };
    ptr = func;

    return i;
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

		if (ppDevice != 0)
		{
			ID3D11Device* pDevice = *ppDevice;

			if (pDevice == 0)
			{
				return result;
			}

      ID3D11DeviceContext* pContext = *ppImmediateContext;

      /*
      if (pContext != nullptr && Original_UnmapHook == 0) {
        DWORD64* vtable = (DWORD64*)pContext;
        vtable = (DWORD64*)vtable[0];

        int ofs = VTableIndex<ID3D11DeviceContext>(&ID3D11DeviceContext::DrawIndexed);
        int ofsother = SeeBits(&ID3D11DeviceContext::DrawIndexed);

        LOGSTRF("dawindexofffawefawffsetty %d %d\n", ofs, ofsother);

        Original_UnmapHook = (UnmapHook)DetourFuncVTable(vtable, (BYTE*)hookedUnmap, 12);
      }
      */
		}

		if (ppSwapChain != 0)
		{
			IDXGISwapChain* pSwapChain = *ppSwapChain;

			if (pSwapChain == 0)
			{
				return result;
			}

			DWORD64* vtable = (DWORD64*)pSwapChain;
			vtable = (DWORD64*)vtable[0];

      int ofs = VTableIndex<IDXGISwapChain>(&IDXGISwapChain::Present);

			Original_PresentHook = (PresentHook)DetourFuncVTable(vtable, (BYTE*)hookedPresent, ofs - 1);
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