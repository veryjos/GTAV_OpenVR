#include "Object.hpp"

#include "Log.hpp"

using namespace OVRInject;

Object::Object(ID3D11Device* device) {
  device_ = device;

  vertex_buffer_ = nullptr;
  index_buffer_ = nullptr;
}

Object::~Object() {
  if (vertex_buffer_)
    vertex_buffer_->Release();

  if (index_buffer_)
    index_buffer_->Release();
}

void Object::InitializeVertices(ObjectVertex *vertices, unsigned long num_vertices, unsigned long *indices, unsigned long num_indices) {
  if (vertex_buffer_ != nullptr ||
    index_buffer_ != nullptr)
    LOGFATALF("Double initialization of Object vertices");

  D3D11_BUFFER_DESC vertex_desc;
  vertex_count_ = num_vertices;

  vertex_desc.Usage = D3D11_USAGE_DEFAULT;
  vertex_desc.ByteWidth = sizeof(ObjectVertex) * num_vertices;
  vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertex_desc.CPUAccessFlags = 0;
  vertex_desc.MiscFlags = 0;
  vertex_desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA vertex_data;

  vertex_data.pSysMem = vertices;
  vertex_data.SysMemPitch = 0;
  vertex_data.SysMemSlicePitch = 0;

  HRESULT result = device_->CreateBuffer(&vertex_desc, &vertex_data, &vertex_buffer_);

  if (FAILED(result))
    LOGFATALF("Failed to create vertex buffer");

  D3D11_BUFFER_DESC index_desc;
  index_count_ = num_indices;

  index_desc.Usage = D3D11_USAGE_DEFAULT;
  index_desc.ByteWidth = sizeof(unsigned long) * num_indices;
  index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  index_desc.CPUAccessFlags = 0;
  index_desc.MiscFlags = 0;
  index_desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA index_data;

  index_data.pSysMem = indices;
  index_data.SysMemPitch = 0;
  index_data.SysMemSlicePitch = 0;

  result = device_->CreateBuffer(&index_desc, &index_data, &index_buffer_);

  if (FAILED(result))
    LOGFATALF("Failed to create index buffer");
}