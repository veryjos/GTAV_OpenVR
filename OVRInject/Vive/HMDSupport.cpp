#include "targetver.h"

#include "HMDSupport.hpp"
#include "HMDRenderer.hpp"

#include "Log.hpp"

#include <cstdio>
#include <dxgi.h>
#include <string>

using namespace OVRInject;
using namespace vr;

HMDSupport::HMDSupport() :
    device_(nullptr), renderer_(new HMDRenderer(this)),
    desktop_mirroring_(true) {
}

HMDSupport::~HMDSupport() {
}

bool HMDSupport::Initialize(IDXGISwapChain* swap_chain, ID3D11Device* device) {
	swap_chain_ = swap_chain;
	device_ = device;

	device_->GetImmediateContext(&device_context_);

  EVRInitError error = VRInitError_None;
  hmd_ = VR_Init(&error, VRApplication_Scene);
  compositor_ = VRCompositor();
  chaperone_ = VRChaperone();

	if (error) {
		hmd_ = NULL;

		LOGSTRF("HMDSupport Initialize error: %s", VR_GetVRInitErrorAsEnglishDescription(error));

		return false;
	}

  // Get the backbuffer so we can initialize frame textures
  // with the same parameters
  ID3D11Texture2D* pBuffer;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);

  // Pass the GTA device context and backbuffer
  renderer_->Initialize(device_context_, pBuffer);

  pBuffer->Release();

	LOGSTRF("Successfully initialized HMDSupport\n");

	return true;
}

void HMDSupport::SubmitFrameTexture(int eye_index, ID3D11Texture2D* texture, const unsigned int& time)
{
  renderer_->SubmitFrameTexture(eye_index, texture, time);
}

HMDSupport* HMDSupport::Singleton() {
  static HMDSupport instance;
   
  return &instance;
}

void HMDSupport::SetFrameHueristic(int frameHuer) {
  renderer_->SetFrameHueristic(frameHuer);
}

void HMDSupport::SetSpinlock(bool enabled) {
  renderer_->SetSpinlock(enabled);
}

void HMDSupport::SetHS(float hs) {
  renderer_->SetHS(hs);
}

void HMDSupport::SetVS(float vs) {
  renderer_->SetVS(vs);
}

void HMDSupport::SetZS(float zs) {
  renderer_->SetZS(zs);
}

void HMDSupport::SyncOnPoses() {
  renderer_->WaitGetPosesSimple();
}

void HMDSupport::SetDesktopMirroring(bool enable) {
  desktop_mirroring_ = enable;
}

bool HMDSupport::GetDesktopMirroring() {
  return desktop_mirroring_;
}