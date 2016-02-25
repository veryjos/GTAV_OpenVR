#pragma once

#include <Windows.h>
#include <d3d11.h>

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
		HMDRenderer();
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
		virtual void PushFrame(ID3D11Texture2D* texture);

		/**
		Stops the rendering thread and kills any resources
		allocated by it
		*/
		virtual void Uninitialize();

	private:
		struct FrameTexture {
			FrameTexture() : texture(nullptr), consumed(false) { };

			ID3D11Texture2D* texture;
			bool consumed;
		};

		HANDLE thread_handle_;
		DWORD thread_id_;

		/**
		A buffer to hold the frame textures that are are passed
		to OpenVR
		*/
		FrameTexture texture_buffer_[NUM_TEXTURES_TO_BUFFER];
	};
};