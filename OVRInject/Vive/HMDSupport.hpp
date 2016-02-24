#pragma once

#include <d3d11.h>
#include "openvr.h"

#include "GL/glew.h"
#include "GL/wglew.h"

#include <vector>

namespace OVRInject
{
	/**
	EyeBuffers hold the handles between D3D and GL textures.
	The texels are shared, so there's minimal copying going on.
	*/
	struct EyeBuffer
	{
		HANDLE dxSharedTexHandle;
		GLuint glTex;
		ID3D11Texture2D* dxTex;
	};

	/**
	HMDSupport is a wrapper class for all-inclusive OpenVR support
	*/
	class HMDSupport
	{
	public:
		HMDSupport();
		virtual ~HMDSupport();

		/**
		Initializes OpenVR
		*/
		bool Initialize(IDXGISwapChain* swapChain, ID3D11Device* device, bool compaitibilityMode);

		/**
		Submits a screen texture to an eye from D3D
		*/
		void SubmitTexture(int eyeIndex, ID3D11Texture2D* texture);

		/**
		Must be called after calling D3D Present
		*/
		void PostPresent();

		/**
		Workaround for 1.0 games
		*/
		void SetOpenGLCompatibilityMode(bool enabled);

	private:
		/**
		In the case that a game uses more than one screen buffer texture,
		this must be called before submitting a frame.
		*/
		void AddPossibleEyeTexture(int eyeId, ID3D11Texture2D* tex);

		/**
		Using a D3D texture as a key, finds an EyeBuffer.
		*/
		EyeBuffer* GetEyeBufferFromTexture(ID3D11Texture2D* tex);
		
		void LogCompositorError(vr::EVRCompositorError err);


		vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];

		ID3D11Device* device;
		IDXGISwapChain* swapChain;

		std::vector<EyeBuffer> leftEyeBuffers;
		std::vector<EyeBuffer> rightEyeBuffers;
		std::vector<HANDLE> usedSharedTextureHandles;

		HANDLE gldxDevice;

		HWND windowHandle;
		vr::IVRSystem* hmd;
		vr::IVRCompositor* compositor;
		bool OpenGLCompatibilityMode;
	};
}