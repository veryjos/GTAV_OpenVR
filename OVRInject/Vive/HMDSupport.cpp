#include "HMDSupport.hpp"
#include <cstdio>

#include <dxgi.h>

#include <string>

#include "../Log.hpp"

using namespace OVRInject;
using namespace vr;

HMDSupport::HMDSupport()
	: device(nullptr)
{

}

HMDSupport::~HMDSupport()
{

}

bool HMDSupport::Initialize(IDXGISwapChain* swapChain, ID3D11Device* device, bool compatibilityMode)
{
	OpenGLCompatibilityMode = compatibilityMode;

	if (OpenGLCompatibilityMode)
	{
		// TODO:
		// GetForegroundWindow() here instead of a proper window creation
		// introduces a possible race condition and point of failure.
		PIXELFORMATDESCRIPTOR pfd;
		HDC dc = GetDC(GetForegroundWindow());
		GLuint PixelFormat = ChoosePixelFormat(dc, &pfd);
		SetPixelFormat(dc, PixelFormat, &pfd);
		HGLRC hRC = wglCreateContext(dc);
		wglMakeCurrent(dc, hRC);

		if (glewInit() != GLEW_OK)
		{
			LOGWNDF("Failed to initialize GLEW");
			return false;
		}

		if (WGLEW_NV_DX_interop)
		{
			gldxDevice = wglDXOpenDeviceNV(device);
			if (gldxDevice == 0)
			{
				LOGWNDF("Failed to initialize NV_DX_interop, this GPU may be unsupported by GTA: V OpenVR");
				return false;
			}
		}
		else
		{
			LOGWNDF("No extension \"NV_DX_interop\" found, this GPU may be unsupported by GTA: V OpenVR");
			return false;
		}
	}

	EVRInitError eError = VRInitError_None;
	hmd = VR_Init(&eError);
	compositor = VRCompositor();

	this->swapChain = swapChain;
	this->device = device;

	if (eError)
	{
		hmd = NULL;

		LOGSTRF("HMDSupport Initialize error: %s", VR_GetVRInitErrorAsEnglishDescription(eError));

		return false;
	}

	LOGSTRF("Successfully initialized HMDSupport");

	return true;
}

void HMDSupport::LogCompositorError(EVRCompositorError err)
{
	if (err != VRCompositorError_None) {
		switch (err)
		{
		case VRCompositorError_DoNotHaveFocus:
			LOGSTRF("Error: VRCompositorError_DoNotHaveFocus\n");
			break;

		case VRCompositorError_IncompatibleVersion:
			LOGSTRF("Error: VRCompositorError_IncompatibleVersion\n");
			break;

		case VRCompositorError_InvalidTexture:
			LOGSTRF("Error: VRCompositorError_InvalidTexture\n");
			break;

		case VRCompositorError_IsNotSceneApplication:
			LOGSTRF("Error: VRCompositorError_IsNotSceneApplication\n");
			break;

		case VRCompositorError_SharedTexturesNotSupported:
			LOGSTRF("Error: VRCompositorError_SharedTexturesNotSupported\n");
			break;

		case VRCompositorError_TextureIsOnWrongDevice:
			LOGSTRF("Error: VRCompositorError_TextureIsOnWrongDevice\n");
			break;

		case VRCompositorError_TextureUsesUnsupportedFormat:
			LOGSTRF("Error: VRCompositorError_TextureUsesUnsupportedFormat\n");
			break;

		default:
			LOGSTRF("Error: Unknown VRCompositorError\n");
			break;
		}
	}
}

void HMDSupport::SubmitTexture(int eyeIndex, ID3D11Texture2D* texture)
{
	if (OpenGLCompatibilityMode)
		AddPossibleEyeTexture(0, texture);

	VREvent_t event;
	while (hmd->PollNextEvent(&event, sizeof(event)))
	{
		switch (event.eventType)
		{
		case VREvent_TrackedDeviceActivated:
			LOGSTRF("Device %u attached.\n", event.trackedDeviceIndex);
			break;

		case VREvent_TrackedDeviceDeactivated:
			LOGSTRF("Device %u detached.\n", event.trackedDeviceIndex);
			break;

		case VREvent_TrackedDeviceUpdated:
			LOGSTRF("Device %u updated.\n", event.trackedDeviceIndex);
			break;
		}
	}

	compositor->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	int m_iValidPoseCount = 0;
	std::string m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (trackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
		}
	}

	if (eyeIndex == 0) {
		// Find the shared texture bound to the texture passed in
		// For now, we just use a monoscopic texture.
		if (OpenGLCompatibilityMode)
		{
			EyeBuffer* eb = GetEyeBufferFromTexture(texture);

			usedSharedTextureHandles.push_back(eb->dxSharedTexHandle);

			wglDXLockObjectsNV(gldxDevice, 1, &eb->dxSharedTexHandle);

			// TODO:
			// Correct aspect ratio in-game

			// OGL mode requires the vertical component flipped
			VRTextureBounds_t bounds;
			bounds.uMax = 0.75f;
			bounds.uMin = 0.25f;
			bounds.vMax = 0.0f;
			bounds.vMin = 1.0f;

			{
				vr::Texture_t leftTex = { (void*)texture, vr::API_DirectX, vr::ColorSpace_Gamma };
				vr::EVRCompositorError err = compositor->Submit(Eye_Left, &leftTex, &bounds);

				if (err)
					LogCompositorError(err);
			}

			{
				vr::Texture_t rightTex = { (void*)texture , vr::API_DirectX, vr::ColorSpace_Gamma };
				vr::EVRCompositorError err = compositor->Submit(Eye_Right, &rightTex, &bounds);

				if (err)
					LogCompositorError(err);
			}
		}
		else
		{
			// TODO:
			// Correct aspect ratio in-game
			VRTextureBounds_t bounds;
			bounds.uMax = 0.75f;
			bounds.uMin = 0.25f;
			bounds.vMax = 1.0f;
			bounds.vMin = 0.0f;

			{
				vr::Texture_t leftTex = { (void*)texture, vr::API_DirectX, vr::ColorSpace_Gamma };
				vr::EVRCompositorError err = compositor->Submit(Eye_Left, &leftTex, &bounds);

				if (err)
					LogCompositorError(err);
			}

			{
				vr::Texture_t rightTex = { (void*)texture , vr::API_DirectX, vr::ColorSpace_Gamma };
				vr::EVRCompositorError err = compositor->Submit(Eye_Right, &rightTex, &bounds);

				if (err)
					LogCompositorError(err);
			}
		}
	}
}

void HMDSupport::PostPresent()
{
	compositor->PostPresentHandoff();

	if (OpenGLCompatibilityMode)
	{
		for (auto handle : usedSharedTextureHandles)
			wglDXUnlockObjectsNV(gldxDevice, 1, &handle);

		usedSharedTextureHandles.clear();
	}
}

void HMDSupport::AddPossibleEyeTexture(int eyeId, ID3D11Texture2D* tex)
{
	// Check if this buffer is already hooked
	if (GetEyeBufferFromTexture(tex))
		return;

	switch (eyeId)
	{
	case 0:
	{
		EyeBuffer buff;
		buff.dxTex = tex;
		glGenTextures(1, &buff.glTex);

		buff.dxSharedTexHandle = wglDXRegisterObjectNV(gldxDevice,
			tex,
			buff.glTex,
			GL_TEXTURE_2D,
			WGL_ACCESS_READ_ONLY_NV);

		leftEyeBuffers.push_back(buff);
		LOGWNDF("Added new texture, count: %d", leftEyeBuffers.size());

		break;
	}

	case 1:
	{
		EyeBuffer buff;
		buff.dxTex = tex;
		glGenTextures(1, &buff.glTex);

		buff.dxSharedTexHandle = wglDXRegisterObjectNV(gldxDevice,
			tex,
			buff.glTex,
			GL_TEXTURE_2D,
			WGL_ACCESS_READ_ONLY_NV);

		rightEyeBuffers.push_back(buff);

		break;
	}
	}
}

EyeBuffer* HMDSupport::GetEyeBufferFromTexture(ID3D11Texture2D* tex)
{
	for (auto &eb : leftEyeBuffers)
	{
		if (eb.dxTex == tex)
			return &eb;
	}

	for (auto &eb : rightEyeBuffers)
	{
		if (eb.dxTex == tex)
			return &eb;
	}

	return NULL;
}