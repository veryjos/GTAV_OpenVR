#include "HMDSupport.hpp"
#include <cstdio>

#include <dxgi.h>

#include <string>

#include "Log.hpp"

using namespace OVRInject;
using namespace vr;

HMDSupport::HMDSupport() :
    device_(nullptr) { }

HMDSupport::~HMDSupport() { }

bool HMDSupport::Initialize(IDXGISwapChain* swap_chain, ID3D11Device* device) {
	swap_chain_ = swap_chain;
	device_ = device;

	device_->GetImmediateContext(&device_context_);

  EVRInitError error = VRInitError_None;
  hmd_ = VR_Init(&error);
  compositor_ = VRCompositor();

	if (error)
	{
		hmd_ = NULL;

		LOGSTRF("HMDSupport Initialize error: %s", VR_GetVRInitErrorAsEnglishDescription(eError));

		return false;
	}

	LOGSTRF("Beginning HMD Polling thread..\n");
	pollHMDPoseThreadHandle = CreateThread(NULL, 0, PollHMDPose, (void*)this, 0, &pollHMDPoseThreadID);

	if (!pollHMDPoseThreadHandle)
	{
		LOGSTRF("Failed to create HMD Polling thread\n");
	}

	LOGSTRF("Successfully initialized HMDSupport\n");

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
	
	currentBuffer = texture;
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

DWORD WINAPI HMDSupport::PollHMDPose(void* param)
{
	HMDSupport* hmdSupport = (HMDSupport*)param;

	while (true) {
		VREvent_t event;
		while (hmdSupport->hmd->PollNextEvent(&event, sizeof(event)))
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

		//compositor->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		// Find the shared texture bound to the texture passed in
		// For now, we just use a monoscopic texture.
		// TODO:
		// Correct aspect ratio in-game
		VRTextureBounds_t bounds;
		bounds.uMax = 0.75f;
		bounds.uMin = 0.25f;
		bounds.vMax = 1.0f;
		bounds.vMin = 0.0f;

		if (hmdSupport->currentBuffer != nullptr)
		{
			{
				vr::Texture_t leftTex = { (void*)hmdSupport->currentBuffer, vr::API_DirectX, vr::ColorSpace_Gamma };
				vr::EVRCompositorError err = hmdSupport->compositor->Submit(Eye_Left, &leftTex, &bounds);

				if (err)
					hmdSupport->LogCompositorError(err);
			}

			{
				vr::Texture_t rightTex = { (void*)hmdSupport->currentBuffer , vr::API_DirectX, vr::ColorSpace_Gamma };
				vr::EVRCompositorError err = hmdSupport->compositor->Submit(Eye_Right, &rightTex, &bounds);

				if (err)
					hmdSupport->LogCompositorError(err);
			}
		}

		vr::EVRCompositorError err = hmdSupport->compositor->WaitGetPoses(hmdSupport->trackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	}

	return 1;
}