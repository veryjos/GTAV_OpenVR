#pragma once

#include "Math/Matrices.h"
#include "openvr.h"

#ifdef OVRINJECT_EXPORTS
#define OVR_API __declspec(dllexport)
#else
#define OVR_API __declspec(dllimport)
#endif

namespace OVRInject
{
  class TrackedController {
  public:
    struct ButtonState {
      ButtonState(bool valid) {
        valid_ = valid;
      };

      float touchX;
      float touchY;

      bool touchContact;
      bool touchJustContacted;
      bool touchJustReleased;

      bool padPressed;
      bool padJustPressed;
      bool padJustReleased;

      bool gripPressed;
      bool gripJustPressed;
      bool gripJustReleased;

      float triggerMargin;

      bool triggerPressed;
      bool triggerJustPressed;
      bool triggerJustReleased;

      bool menuPressed;
      bool menuJustPressed;
      bool menuJustReleased;

      bool valid_;
    };

    OVR_API TrackedController();
    OVR_API ~TrackedController();

    void SetMatrix(const Matrix4& matrix);
    OVR_API Matrix4 GetMatrix();

    OVR_API XMFLOAT3 GetPosition();
    OVR_API XMFLOAT3 GetRotation(float rx = 0.0f, float ry = 0.0f, float rz = 0.0f);

    void SetDeviceIndex(vr::TrackedDeviceIndex_t device_index);

    OVR_API void UpdateButtonState();
    OVR_API ButtonState GetButtonState();
   
  private:
    uint32_t last_packet_;
    ButtonState button_state_;
    vr::TrackedDeviceIndex_t device_index_;
    Matrix4 matrix_;
  };
}