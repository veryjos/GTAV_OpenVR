#pragma once
#include "targetver.h"


#include <d3d11.h>
#include <dxgi.h>

#include <DirectXMath.h>
#include "Vive/Math/Matrices.h"

using namespace DirectX;

namespace OVRInject {
  struct ObjectVertex {
    XMFLOAT3 position;
    XMFLOAT2 texcoord;
  };

  /**
  Object that will be rendered.
  */
  class Object {
    friend class RenderScene;

  public:
    Object(ID3D11Device *device);
    virtual ~Object();

    XMFLOAT3 position;
    XMFLOAT3 focus;
    XMFLOAT3 up;

    XMMATRIX GetWorldMatrix() {
      Matrix4 translation;
      translation.translate(position.x, position.y, position.z);

      XMMATRIX rot = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(position.x - focus.x, position.y - focus.y, position.z - focus.z, 1.0f), XMVectorSet(up.x, up.y, up.z, 1.0f));
      XMMATRIX result = XMMatrixMultiply(translation.getDXMatrix(), rot);

      return result;
    };

    void InitializeVertices(ObjectVertex *vertices, unsigned long num_vertices, unsigned long *indices, unsigned long num_indices);
    bool SetTexture(ID3D11Texture2D* texture);
    
    ID3D11ShaderResourceView* GetTexture();

  private:

    ID3D11Device *device_;
    ID3D11Buffer *vertex_buffer_, *index_buffer_;
    int vertex_count_, index_count_;

    ID3D11ShaderResourceView* texture_resource_view_;
  };
}