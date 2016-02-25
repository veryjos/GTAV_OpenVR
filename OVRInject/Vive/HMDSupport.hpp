#pragma once

#include <d3d11.h>
#include "openvr.h"

#include "GL/glew.h"
#include "GL/wglew.h"

#include <vector>

namespace OVRInject
{
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
		bool Initialize(IDXGISwapChain* swap_chain, ID3D11Device* device);

		/**
		Submits a screen texture to an eye from D3D
		*/
		void SubmitTexture(int eye_index, ID3D11Texture2D* texture);

		/**
		Must be called after calling D3D Present
		*/
		void PostPresent();

	private:
		ID3D11Device* device_;
		ID3D11DeviceContext* device_context_;

		IDXGISwapChain* swap_chain_;

		vr::IVRSystem* hmd_;
		vr::IVRCompositor* compositor_;
	};
}