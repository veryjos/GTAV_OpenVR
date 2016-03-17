#pragma once

#include "Object.hpp"

namespace OVRInject {
  struct MatrixBuffer
  {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
  };

  class CameraObject : public Object {
  public:
    CameraObject(ID3D11Device *device) : Object(device) {};
    virtual ~CameraObject() {};

    XMMATRIX GetViewMatrix() {
      return XMMATRIX();
    };
    
    XMMATRIX GetProjectionMatrix() {
      return XMMATRIX();
    };

    float fov;
  };
}