#include "HMDRenderer.hpp"

#include "Scene/RenderScene.hpp"

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

  ID3D11Texture2D** eye_textures[] = { &rt_texture_left_, &rt_texture_right_ };
  ID3D11RenderTargetView** eye_targets[] = { &rt_view_left_, &rt_view_right_ };

  ID3D11Texture2D** depth_textures[] = { &depth_texture_left_, &depth_texture_right_ };
  ID3D11DepthStencilState** depth_states[] = { &depth_state_left_, &depth_state_right_ };
  ID3D11DepthStencilView** depth_views[] = { &depth_view_left_, &depth_view_right_ };

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
    texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texture_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    HRESULT result = device_->CreateTexture2D(&texture_desc, nullptr,
                                              eye_textures[i]);

    LOGSTRF("HMDRenderer successfully created render texture %d\n", i);

    if (FAILED(result))
      LOGFATALF("HMDRenderer failed to create eye texture %d\nError: %d", i, result);

    D3D11_RENDER_TARGET_VIEW_DESC target_desc;
    target_desc.Format = texture_desc.Format;
    target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    target_desc.Texture2D.MipSlice = 0;

    result = device_->CreateRenderTargetView(*eye_textures[i], &target_desc,
                                             eye_targets[i]);

    if (FAILED(result))
      LOGFATALF("HMDRenderer failed to create eye render target %d\nError: %d", i, result);

    LOGSTRF("HMDRenderer successfully created render target %d\n", i);

    LOGSTRF("HMDRenderer creating depth texture %d\n", i);

    D3D11_TEXTURE2D_DESC depth_desc;
    memset(&depth_desc, 0, sizeof(D3D11_TEXTURE2D_DESC));

    depth_desc.Width = texture_width;
    depth_desc.Height = texture_height;
    depth_desc.MipLevels = 1;
    depth_desc.ArraySize = 1;
    depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_desc.SampleDesc.Count = 1;
    depth_desc.SampleDesc.Quality = 0;
    depth_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depth_desc.CPUAccessFlags = 0;
    depth_desc.MiscFlags = 0;

    result = device_->CreateTexture2D(&depth_desc, NULL, depth_textures[i]);

    if (FAILED(result))
      LOGFATALF("HMDRenderer failed to create eye depth texture %d\nError: %d", i, result);

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
    memset(&depth_stencil_desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));

    // Set up the description of the stencil state.
    depth_stencil_desc.DepthEnable = true;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

    depth_stencil_desc.StencilEnable = true;
    depth_stencil_desc.StencilReadMask = 0xFF;
    depth_stencil_desc.StencilWriteMask = 0xFF;

    depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    result = device_->CreateDepthStencilState(&depth_stencil_desc, depth_states[i]);

    if (FAILED(result))
      LOGFATALF("HMDRenderer failed to create stencil state %d\nError: %d", i, result);

    D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_desc;
    memset(&depth_view_desc, 0, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

    depth_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depth_view_desc.Texture2D.MipSlice = 0;

    result = device_->CreateDepthStencilView(*depth_textures[i], &depth_view_desc, depth_views[i]);

    if (FAILED(result))
      LOGFATALF("HMDRenderer failed to create stencil view %d\nError: %d", i, result);
  }
}

void HMDRenderer::InitializeFrameTextures(ID3D11Texture2D* base_texture_) {

}

void HMDRenderer::InitializeFrameRenderModels() {

}

static inline void LogCompositorError(EVRCompositorError error) {
  if (error != VRCompositorError_None) {
    switch (error) {
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

  RenderScene scene(device_);
  scene.Initialize();

  bool rendering_ = true;
  while (rendering_) {
    // Main render loop

    // TODO:
    // Consume textures from queue

    scene.RenderFrame(&rt_view_left_, &depth_view_left_, &depth_state_left_);
    scene.RenderFrame(&rt_view_right_, &depth_view_right_, &depth_state_right_);

    device_context_->Flush();

    {
      vr::Texture_t leftTex = { (void*)rt_texture_left_, vr::API_DirectX, vr::ColorSpace_Gamma };
      vr::EVRCompositorError err = compositor->Submit(Eye_Left, &leftTex);

      if (err)
        LogCompositorError(err);
    }

    {
      vr::Texture_t rightTex = { (void*)rt_texture_right_ , vr::API_DirectX, vr::ColorSpace_Gamma };
      vr::EVRCompositorError err = compositor->Submit(Eye_Right, &rightTex);

      if (err)
        LogCompositorError(err);
    }

    // Get device poses / sync with renderer
    compositor->WaitGetPoses(&poses[0], k_unMaxTrackedDeviceCount, nullptr, 0);
  }

  scene.Uninitialize();

  device_->Release();
  device_context_->Release();
}