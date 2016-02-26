#include "HMDRenderer.hpp"

#include "openvr.h"
#include "Log.hpp"

using namespace vr;
using namespace OVRInject;

HMDRenderer::HMDRenderer(HMDSupport* hmd_support) :
    hmd_support_(hmd_support),
    rt_texture_left_(nullptr), rt_view_left_(nullptr),
    rt_texture_right_(nullptr), rt_view_right_(nullptr) {
}

HMDRenderer::~HMDRenderer() {
}

void HMDRenderer::Initialize() {
	// Begin the renderer thread
	thread_handle_ = CreateThread(NULL, 0, &RenderThreadEntry, (void*)this, 0, &thread_id_);
}

void HMDRenderer::SubmitFrameTexture(const int& eye_index, const ID3D11Texture2D* texture) {
  // TODO:
  // Find unused texture
  // Add to queue
}

void HMDRenderer::Uninitialize() {
  // TODO:
  // Uninitialize everything
}

DWORD WINAPI HMDRenderer::RenderThreadEntry(void* param) {
  HMDRenderer* renderer = reinterpret_cast<HMDRenderer*>(param);

  renderer->RenderThread();

  return 0;
}

void HMDRenderer::InitializeRenderTargets() {
  // Get the values that OpenVR wants us to use
  IVRSystem* hmd = hmd_support_->get_hmd();
  IVRCompositor* compositor = hmd_support_->get_compositor();

  ID3D11Texture2D* eye_textures[] = { rt_texture_left_, rt_texture_right_ };
  ID3D11RenderTargetView* eye_targets[] = { rt_view_left_, rt_view_right_ };

  // Create texture and render target view for each eye
  for (uint32_t i = 0; i < sizeof(eye_textures) / sizeof(eye_textures[0]); ++i) {
    uint32_t texture_width, texture_height;
    hmd->GetRecommendedRenderTargetSize(&texture_width, &texture_height);

    D3D11_TEXTURE2D_DESC texture_desc;
    memset(&texture_desc, 0, sizeof(D3D11_TEXTURE2D_DESC));

    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Width = texture_width;
    texture_desc.Height = texture_height;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    texture_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    HRESULT result = device_->CreateTexture2D(&texture_desc, nullptr,
                                              &eye_textures[i]);

    if (FAILED(result))
      LOGFATALF("HMDRenderer failed to create eye texture %d\nError: %d", i, result);

    D3D11_RENDER_TARGET_VIEW_DESC target_desc;
    target_desc.Format = texture_desc.Format;
    target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    target_desc.Texture2D.MipSlice = 0;

    result = device_->CreateRenderTargetView(eye_textures[i], &target_desc,
                                             &eye_targets[i]);

    if (FAILED(result))
      LOGFATALF("HMDRenderer failed to create eye render target %d\nError: %d", i, result);

    LOGSTRF("HMDRenderer successfully created render target %d\n", i);
  }
}

void HMDRenderer::InitializeFrameTextures(ID3D11Texture2D* base_texture_) {

}

void HMDRenderer::RenderThread() {
  // Initialize D3D11 state
  HRESULT result = 
      D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr,
                        0, D3D11_SDK_VERSION, &device_, 0, &device_context_);

  if (FAILED(result))
    LOGFATALF("HMDRenderer device creation error: %d\n", result);

  InitializeRenderTargets();

  IVRSystem* hmd = hmd_support_->get_hmd();
  IVRCompositor* compositor = hmd_support_->get_compositor();

  // Array to hold device poses
  TrackedDevicePose_t poses[k_unMaxTrackedDeviceCount];

  bool rendering_ = true;
  while (rendering_) {
    // Main render loop

    // TODO:
    // Consume textures from queue

    // TODO:
    // Draw onto rendertargets
    device_context_->Flush();

    // TODO:
    // Present rendertargets to OpenVR

    // Get device poses / sync with renderer
    compositor->WaitGetPoses(&poses[0], k_unMaxTrackedDeviceCount, nullptr, 0);
  }

  device_->Release();
  device_context_->Release();
}