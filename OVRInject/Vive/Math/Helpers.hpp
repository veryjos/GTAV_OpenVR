#pragma once
#include "targetver.h"

#include <openvr.h>
#include "Matrices.h"

using namespace vr;

static inline Matrix4 ConvertSteamVRMatrixToMatrix4(const HmdMatrix34_t &matPose) {
  Matrix4 matrixObj(
    matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
    matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
    matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
    matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
  );

  return matrixObj;
}