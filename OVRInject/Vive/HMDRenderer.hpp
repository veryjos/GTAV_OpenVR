#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "HMDSupport.hpp"

#include "atomicops.h"
#include "readerwriterqueue.h"

#include "Math/Helpers.hpp"

#include <future>

// ReaderWriterQueue's namespace
using namespace moodycamel;

namespace OVRInject
{
  class RenderScene;
#define NUM_TEXTURES_TO_BUFFER 4

	/**
	Creates a Direct3D Device and rendering context for delivering
	frames to OpenVR while maintaining a constant target framerate
	for head tracking.
	*/
	class HMDRenderer {
	public:
		HMDRenderer(HMDSupport* hmd_support);
		virtual ~HMDRenderer();

		/**
		Starts the renderer thread
		*/
		virtual void Initialize(ID3D11DeviceContext* gta_context, ID3D11Texture2D* backbuffer_texture);

		/**
		Safe to calls this from outside the rendering thread

		Pushes a frame to the rendering thread to update the virtual
		screen. This call only blocks to copy the texture into a
		texture on our buffer list
		*/
		virtual void SubmitFrameTexture(const int& eye_index, ID3D11Texture2D* texture, const unsigned int& time);

    void SetFrameHueristic(int id) {
      frame_hueristic_ = id;
    };

    void SetSpinlock(bool enabled) {
      spinlock_ = enabled;
    };

    void SetHS(float hs);
    void SetVS(float vs);
    void SetZS(float zs);
    void SetFS(float zs);

    void WaitGetPoses();
    void WaitGetPosesSimple();

		/**
		Stops the rendering thread and kills any resources
		allocated by it
		*/
		virtual void Uninitialize();

	private:
    /**
    The HMDSupport instance that created this renderer
    */
    HMDSupport* hmd_support_;

    /**
    Holds references to textures that are submitted to OpenVR,
    and if they're been consumed or not.
    */
		struct FrameTexture {
			FrameTexture() : texture(nullptr), consumed(false), time(UINT_MAX){ };

			ID3D11Texture2D* texture;
      ID3D11Texture2D* shared_texture;
			bool consumed;
      unsigned int time;
		};

    /**
    Initializes the render targets for each eye
    */
    void InitializeRenderTargets();

    /**
    Initializes the shared resources (FIFO's shared textures)
    */
    void InitializeSharedResources();

    /**
    Initializes the frame textures, using the texture passed in
    as a template.
    */
    void InitializeFrameTextures(ID3D11Texture2D* base_texture);

    void InitializeFrameRenderModels();

    void STInit();
    void STPush(FrameTexture* tex);
    void STEnd();

    static bool RenderThreadEntry(HMDRenderer* renderer);
    void RenderThread();

		std::thread thread_handle_;
		DWORD thread_id_;

		/**
		A buffer to hold the frame textures that are are passed to OpenVR
		*/
		FrameTexture texture_buffer_[NUM_TEXTURES_TO_BUFFER];
    
    /**
    Lockless queue for safe multithreaded consumption of textures
    */
    ReaderWriterQueue<FrameTexture*> texture_queue_;
    std::vector<FrameTexture*> sorted_frame_textures_;

    bool spinlock_;
    int frame_hueristic_;

    ID3D11Device* gta_device_;
    ID3D11DeviceContext* gta_context_;

    IDXGISwapChain* swap_chain_;
    ID3D11Device* device_;
    ID3D11DeviceContext* device_context_;

    ID3D11Texture2D* rt_texture_left_;
    ID3D11RenderTargetView* rt_view_left_;

    ID3D11Texture2D* rt_texture_right_;
    ID3D11RenderTargetView* rt_view_right_;

    ID3D11Texture2D* depth_texture_left_;
    ID3D11DepthStencilState* depth_state_left_;
    ID3D11DepthStencilView* depth_view_left_;

    ID3D11Texture2D* depth_texture_right_;
    ID3D11DepthStencilState* depth_state_right_;
    ID3D11DepthStencilView* depth_view_right_;

    D3D11_VIEWPORT viewport_;

    FrameTexture* last_frame_texture_;

    float hs_;
    float vs_;
    float zs_;
    float fs_;

    RenderScene* scene_;

    XMMATRIX view_mat_;
    XMMATRIX proj_mat_left_;
    XMMATRIX proj_mat_right_;

    // Array to hold device poses
    TrackedDevicePose_t poses_[k_unMaxTrackedDeviceCount];

    ID3D11ShaderResourceView* shader_;

    std::future<bool> posesReady;
    std::thread posesThread;
	};
};