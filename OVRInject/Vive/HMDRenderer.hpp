#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "HMDSupport.hpp"

#include "atomicops.h"
#include "readerwriterqueue.h"

// ReaderWriterQueue's namespace
using namespace moodycamel;

namespace OVRInject
{
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
		virtual void Initialize();

		/**
		Safe to calls this from outside the rendering thread

		Pushes a frame to the rendering thread to update the virtual
		screen. This call only blocks to copy the texture into a
		texture on our buffer list
		*/
		virtual void SubmitFrameTexture(const int& eye_index, const ID3D11Texture2D* texture);

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
			FrameTexture() : texture(nullptr), consumed(false) { };

			ID3D11Texture2D* texture;
			bool consumed;
		};

    /**
    Initializes the render targets for each eye
    */
    void InitializeRenderTargets();

    /**
    Initializes the frame textures, using the texture passed in
    as a template.
    */
    void InitializeFrameTextures(ID3D11Texture2D* base_texture_);

    void InitializeFrameRenderModels();

    static DWORD WINAPI RenderThreadEntry(void* param);
    void RenderThread();

		HANDLE thread_handle_;
		DWORD thread_id_;

		/**
		A buffer to hold the frame textures that are are passed to OpenVR
		*/
		FrameTexture texture_buffer_[NUM_TEXTURES_TO_BUFFER];
    
    /**
    Lockless queue for safe multithreaded consumption of textures
    */
    ReaderWriterQueue<FrameTexture> texture_queue_;

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

    ID3D11ShaderResourceView* shader_;
	};
};