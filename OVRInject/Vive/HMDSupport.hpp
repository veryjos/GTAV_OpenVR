#pragma once

#include "openvr.h"

#include "TrackedController.hpp"

#include <vector>
#include <d3d11.h>

#include "Math/Helpers.hpp"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

using namespace vr;

#ifdef OVRINJECT_EXPORTS
#define OVR_API __declspec(dllexport)
#else
#define OVR_API __declspec(dllimport)
#endif

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
    friend class HMDRenderer;

	public:
    OVR_API HMDSupport();
    OVR_API virtual ~HMDSupport();

		/**
		Initializes OpenVR
		*/
    OVR_API bool Initialize(IDXGISwapChain* swap_chain, ID3D11Device* device);

		/**
		Submits a screen texture to an eye from D3D. Safe to call from any
    thread.
		*/
    OVR_API void SubmitFrameTexture(int eye_index, ID3D11Texture2D* texture, const unsigned int& time);

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

    /**
    Gets the left-handed tracked controller
    */
    inline TrackedController* GetLeftHand() {
      return &left_controller_;
    };

    /**
    Gets the right-handed tracked controller
    */
    inline TrackedController* GetRightHand() {
      return &right_controller_;
    };

    OVR_API void HMDSupport::SetFrameHueristic(int frameHuer);
    OVR_API void HMDSupport::SetSpinlock(bool enabled);

    OVR_API void HMDSupport::SetHS(float hs);
    OVR_API void HMDSupport::SetVS(float vs);
    OVR_API void HMDSupport::SetZS(float vs);

    inline HMDRenderer* GetRenderer() {
      return renderer_;
    };

    /**
    Gets the matrix for the HMD
    */
    inline Matrix4 GetHeadMatrix() const {
      return hmd_matrix_;
    };

    inline XMFLOAT3 GetHeadPosition(float yaw) {
      auto pos = hmd_matrix_;
      
      Matrix4 yawMat;
      yawMat.rotateY(yaw);

      pos = yawMat * pos;

      pos.invert();

      XMVECTOR result = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), pos.transpose().getDXMatrix());

      XMFLOAT3 position;

      position.x = XMVectorGetX(result);
      position.y = XMVectorGetY(result);
      position.z = XMVectorGetZ(result);

      return position;
    };

    inline XMFLOAT3 GetEyePosition(EVREye eye) {
      auto eyePos = ConvertSteamVRMatrixToMatrix4(hmd_->GetEyeToHeadTransform(eye)).invert().GetPosition();

      auto pos = hmd_matrix_;
      pos.invert();

      XMVECTOR result = XMVector3Transform(XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 1.0f), pos.transpose().getDXMatrix());

      XMFLOAT3 position;

      position.x = XMVectorGetX(result);
      position.y = XMVectorGetY(result);
      position.z = XMVectorGetZ(result);

      return position;
    };

    inline XMFLOAT3 GetHeadForwardVector() {
      XMFLOAT3 forward;

      auto m = hmd_matrix_.getDXMatrix();

      forward.x = -XMVectorGetX((m.r[2]));
      forward.y = -XMVectorGetY((m.r[2]));
      forward.z = -XMVectorGetZ((m.r[2]));

      return forward;
    };

    inline XMFLOAT3 GetHeadUpVector() {
      Matrix4 mat(hmd_matrix_);
      mat = mat.invert();

      Matrix4 matc = mat;

      // Calculate position
      XMVECTOR result = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), matc.transpose().getDXMatrix());

      XMFLOAT3 position;

      position.x = XMVectorGetX(result);
      position.y = XMVectorGetY(result);
      position.z = XMVectorGetZ(result);

      Matrix4 matNoTranslate = mat;
      matNoTranslate = matNoTranslate.translate(-position.x, -position.y, -position.z);

      XMFLOAT3 up;
      XMVECTOR upResult = XMVector3Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), matNoTranslate.transpose().getDXMatrix());

      upResult = XMVector3Normalize(upResult);

      up.x = XMVectorGetX(upResult);
      up.y = XMVectorGetY(upResult);
      up.z = XMVectorGetZ(upResult);

      return up;
    };

    inline XMFLOAT3 GetHeadRotation() {
      auto forward = GetHeadForwardVector();
      auto up = GetHeadUpVector();

      auto altForward = forward;
      auto altUp = up;

      XMFLOAT3 globalUp;
      globalUp.x = 0.0f;
      globalUp.y = 1.0f;
      globalUp.z = 0.0f;

      auto right = XMVector3Cross(XMLoadFloat3(&altForward), XMLoadFloat3(&altUp));

      XMFLOAT3 rotation;

      rotation.x = -asin(-forward.y);
      rotation.y = -atan2(XMVectorGetY(right), up.y);
      rotation.z = atan2(forward.x, forward.z);

      rotation.x = XMConvertToDegrees(rotation.x);
      rotation.y = XMConvertToDegrees(rotation.y);
      rotation.z = XMConvertToDegrees(rotation.z);

      return rotation;
    };

    inline HmdQuad_t GetPlaySpaceSize() {
      HmdQuad_t quad;

      chaperone_->GetPlayAreaRect(&quad);

      return quad;
    };

    OVR_API static HMDSupport* Singleton();

    OVR_API void SyncOnPoses();

    OVR_API void SetDesktopMirroring(bool enable);
    OVR_API bool GetDesktopMirroring();

	private:
    HMDRenderer* renderer_;

    Matrix4 hmd_matrix_;

    TrackedController left_controller_;
    TrackedController right_controller_;

		IVRSystem* hmd_;
    IVRCompositor* compositor_;
    IVRChaperone* chaperone_;

    ID3D11Device* device_;
    ID3D11DeviceContext* device_context_;

    IDXGISwapChain* swap_chain_;
    bool desktop_mirroring_;
	};
}