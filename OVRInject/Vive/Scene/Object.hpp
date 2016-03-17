#pragma once

#include <d3d11.h>
#include <dxgi.h>

#include <DirectXMath.h>

using namespace DirectX;

namespace OVRInject {
  struct ObjectVertex {
    XMFLOAT3 position;
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
    XMFLOAT4 rotation;

    XMMATRIX GetWorldMatrix() {
      return XMMATRIX();
    };

    void InitializeVertices(ObjectVertex *vertices, unsigned long num_vertices, unsigned long *indices, unsigned long num_indices);

  private:

    XMMATRIX worldMatrix;

    ID3D11Device *device_;
    ID3D11Buffer *vertex_buffer_, *index_buffer_;
    int vertex_count_, index_count_;
  };
}