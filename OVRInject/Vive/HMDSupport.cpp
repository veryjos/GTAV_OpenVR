#include "HMDSupport.hpp"
#include "HMDRenderer.hpp"

#include "Log.hpp"

#include <cstdio>
#include <dxgi.h>
#include <string>

using namespace OVRInject;
using namespace vr;

HMDSupport::HMDSupport() :
    device_(nullptr), renderer_(new HMDRenderer(this)) {
}

HMDSupport::~HMDSupport() {
}

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

		LOGSTRF("HMDSupport Initialize error: %s", VR_GetVRInitErrorAsEnglishDescription(error));

		return false;
	}

  renderer_->Initialize();

	LOGSTRF("Successfully initialized HMDSupport\n");

	return true;
}

void HMDSupport::SubmitFrameTexture(int eye_index, ID3D11Texture2D* texture)
{
  renderer_->SubmitFrameTexture(eye_index, texture);
}