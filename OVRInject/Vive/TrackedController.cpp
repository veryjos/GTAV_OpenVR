#include "targetver.h"

#include "TrackedController.hpp"
#include "HMDSupport.hpp"

#include "Log.hpp"

using namespace OVRInject;

TrackedController::TrackedController() : device_index_(-1), button_state_(true), last_packet_(0) {
}

TrackedController::~TrackedController() {
}

void TrackedController::SetMatrix(const Matrix4& matrix) {
  matrix_ = matrix;
}

XMFLOAT3 TrackedController::GetPosition() {
  return matrix_.GetPosition();
}

XMFLOAT3 TrackedController::GetRotation(float rx, float ry, float rz) {
  return matrix_.GetAngles(rx, ry, rz);
}

Matrix4 TrackedController::GetMatrix() {
  return matrix_;
}

void TrackedController::SetDeviceIndex(vr::TrackedDeviceIndex_t index) {
  device_index_ = index;
}

void TrackedController::UpdateButtonState() {
  if (device_index_ == -1)
    return;

  auto hmd = HMDSupport::Singleton()->get_hmd();

  vr::VRControllerState_t state;

  if (!hmd->GetControllerState(device_index_, &state)) {
    button_state_.valid_ = false;
    return;
  }
  
  button_state_.valid_ = true;

  // Grip button
  button_state_.gripJustPressed = false;
  button_state_.gripJustReleased = false;

  if (state.ulButtonPressed & (1ULL << ((int)k_EButton_Grip)))
    button_state_.gripJustPressed = !button_state_.gripPressed;
  else
    button_state_.gripJustReleased = button_state_.gripPressed;

  button_state_.gripPressed = (state.ulButtonPressed & (1ULL << ((int)k_EButton_Grip))) > 0;

  // Pad button
  button_state_.padJustPressed = false;
  button_state_.padJustReleased = false;

  if (state.ulButtonPressed & (1ULL << ((int)k_EButton_SteamVR_Touchpad)))
    button_state_.padJustPressed = !button_state_.padPressed;
  else
    button_state_.padJustReleased = button_state_.padPressed;

  button_state_.padPressed = (state.ulButtonPressed & (1ULL << ((int)k_EButton_SteamVR_Touchpad))) > 0;

  // Pad touched
  button_state_.touchJustContacted = false;
  button_state_.touchJustReleased = false;

  if (state.ulButtonTouched & (1ULL << ((int)k_EButton_SteamVR_Touchpad)))
    button_state_.touchJustContacted = !button_state_.touchContact;
  else
    button_state_.touchJustReleased = button_state_.touchContact;

  button_state_.touchContact = (state.ulButtonTouched & (1ULL << ((int)k_EButton_SteamVR_Touchpad))) > 0;

  // Touchpad coordinates
  button_state_.touchX = state.rAxis[0].x;
  button_state_.touchY = state.rAxis[0].y;

  // Trigger button
  button_state_.triggerJustPressed = false;
  button_state_.triggerJustReleased = false;

  if (state.ulButtonPressed & (1ULL << ((int)k_EButton_SteamVR_Trigger)))
    button_state_.triggerJustPressed = !button_state_.triggerPressed;
  else
    button_state_.triggerJustReleased = button_state_.triggerPressed;

  button_state_.triggerPressed = (state.ulButtonPressed & (1ULL << ((int)k_EButton_SteamVR_Trigger))) > 0;

  // Trigger margin
  button_state_.triggerMargin = state.rAxis[1].x;

  // Menu button
  button_state_.menuJustPressed = false;
  button_state_.menuJustReleased = false;

  if (state.ulButtonPressed & (1ULL << ((int)k_EButton_ApplicationMenu)))
    button_state_.menuJustPressed = !button_state_.menuPressed;
  else
    button_state_.menuJustReleased = button_state_.menuPressed;

  button_state_.menuPressed = (state.ulButtonPressed & (1ULL << ((int)k_EButton_ApplicationMenu))) > 0;
}

TrackedController::ButtonState TrackedController::GetButtonState() {
  if (device_index_ == -1)
    return TrackedController::ButtonState(false);

  return button_state_;
}