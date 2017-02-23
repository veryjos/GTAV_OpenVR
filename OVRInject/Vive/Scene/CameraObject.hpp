#pragma once
#include "targetver.h"


#include "Object.hpp"
#include "Log.hpp"

namespace OVRInject {
  struct MatrixBuffer
  {
    XMMATRIX mat_mvp;
  };

  class CameraObject : public Object {
  public:
    CameraObject(ID3D11Device *device) : Object(device) {};
    virtual ~CameraObject() {};

    void SetViewMatrix(XMMATRIX &view_matrix) {
      view_matrix_ = view_matrix;

      Matrix4 mat(view_matrix);
      mat = mat.invert();

      Matrix4 matc = mat;

      // Calculate position
      XMVECTOR result = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), matc.transpose().getDXMatrix());

      position.x = XMVectorGetX(result);
      position.y = XMVectorGetY(result);
      position.z = XMVectorGetZ(result);

      Matrix4 matNoTranslate = mat;
      matNoTranslate = matNoTranslate.translate(-position.x, -position.y, -position.z);

      // Calculate up vector
      XMVECTOR upResult = XMVector3Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), matNoTranslate.transpose().getDXMatrix());

      upResult = XMVector3Normalize(upResult);

      up.x = XMVectorGetX(upResult);
      up.y = XMVectorGetY(upResult);
      up.z = XMVectorGetZ(upResult);
    };

    XMMATRIX GetViewMatrix() {
      return view_matrix_;
    };
    
    void SetProjectionMatrix(XMMATRIX &projection_matrix) {
      projection_matrix_ = projection_matrix;
    };

    XMMATRIX GetProjectionMatrix() {
      return projection_matrix_;
    };

    void SetEyeMatrix(XMMATRIX &eye_matrix) {
      eye_matrix_ = eye_matrix;
    };

    XMMATRIX GetEyeMatrix() {
      return eye_matrix_;
    };

    XMMATRIX GetMVEPMatrix(XMMATRIX &model_matrix) {
      return projection_matrix_ * eye_matrix_ * view_matrix_ * model_matrix;
    };

    XMFLOAT3 GetForwardVector() {
      XMFLOAT3 forward;

      forward.x = -XMVectorGetX((view_matrix_.r[2]));
      forward.y = -XMVectorGetY((view_matrix_.r[2]));
      forward.z = -XMVectorGetZ((view_matrix_.r[2]));

      return forward;
    };

    

  private:
    XMMATRIX projection_matrix_;
    XMMATRIX view_matrix_;
    XMMATRIX eye_matrix_;
  };
}