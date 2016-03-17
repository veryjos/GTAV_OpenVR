#pragma once

#include "openvr.h"

#include <vector>
#include <d3d11.h>

using namespace vr;

namespace OVRInject
{
  // Forward declaration required here, circular
  // dependency
  class HMDRenderer;

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
		Submits a screen texture to an eye from D3D. Safe to call from any
    thread.
		*/
		void SubmitFrameTexture(int eye_index, ID3D11Texture2D* texture);

    /**
    Gets the OpenVR HMD instantiated by this support class
    */
    inline IVRSystem* get_hmd() const {
      return hmd_;
    };

    /**
    Gets the OpenVR HMD instantiated by this support class
    */
    inline IVRCompositor* get_compositor() const {
      return compositor_;
    };

	private:
    HMDRenderer* renderer_;

		IVRSystem* hmd_;
    IVRCompositor* compositor_;

    ID3D11Device* device_;
    ID3D11DeviceContext* device_context_;

    IDXGISwapChain* swap_chain_;
	};
}