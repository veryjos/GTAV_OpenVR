#pragma once
#include "targetver.h"


#include <d3d11.h>
#include <d3dcompiler.h>

#include "CameraObject.hpp"
#include "Log.hpp"

namespace OVRInject {
  class SimpleShader {
  public:
    SimpleShader(ID3D11Device* device) {
      HRESULT result;
      ID3DBlob* error = 0;
      ID3DBlob* vertex_shader_buffer = 0;
      ID3DBlob* pixel_shader_buffer = 0;

      device_ = device;
      device->GetImmediateContext(&context_);

      result = D3DCompileFromFile(L"gtaovr/simple.vs", nullptr, nullptr, "SimpleVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertex_shader_buffer, &error);

      if (FAILED(result)) {
        LOGWNDF("Failed to compile shader from gtaovr/simple.vs 0x%p %d", error->GetBufferPointer(), error->GetBufferSize());

        char* err = (char*)error->GetBufferPointer();
        LOGFATALF("Err: %S", err);
      }

      result = D3DCompileFromFile(L"gtaovr/simple.ps", nullptr, nullptr, "SimplePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixel_shader_buffer, &error);
      
      if (FAILED(result)) {
        LOGWNDF("Failed to compile shader from gtaovr/simple.ps 0x%p %d", error->GetBufferPointer(), error->GetBufferSize());

        char* err = (char*)error->GetBufferPointer();
        LOGFATALF("Err: %S", err);
      }

      result = device->CreateVertexShader(vertex_shader_buffer->GetBufferPointer(), vertex_shader_buffer->GetBufferSize(), nullptr, &vertex_shader_);

      if (FAILED(result))
        LOGFATALF("Failed to create vertex shader from gtaovr/simple.vs");

      result = device->CreatePixelShader(pixel_shader_buffer->GetBufferPointer(), pixel_shader_buffer->GetBufferSize(), nullptr, &pixel_shader_);

      if (FAILED(result))
        LOGFATALF("Failed to create pixel shader from gtaovr/simple.ps");

      D3D11_INPUT_ELEMENT_DESC polygon_layout[] =
      {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
      };

      unsigned int num_elements = sizeof(polygon_layout) / sizeof(polygon_layout[0]);

      result = device->CreateInputLayout(polygon_layout, num_elements, vertex_shader_buffer->GetBufferPointer(), vertex_shader_buffer->GetBufferSize(), &layout_);

      if (FAILED(result))
        LOGFATALF("Failed to create input layout from gtaovr/simple");

      D3D11_BUFFER_DESC matrix_buffer_desc;

      matrix_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
      matrix_buffer_desc.ByteWidth = sizeof(MatrixBuffer);
      matrix_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      matrix_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      matrix_buffer_desc.MiscFlags = 0;
      matrix_buffer_desc.StructureByteStride = 0;

      result = device->CreateBuffer(&matrix_buffer_desc, nullptr, &matrix_buffer_);

      if (FAILED(result))
        LOGFATALF("Failed to create matrix buffer for gtaovr/simple");

      D3D11_SAMPLER_DESC sampler_desc;

      sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.MipLODBias = 0.0f;
      sampler_desc.MaxAnisotropy = 1;
      sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
      sampler_desc.BorderColor[0] = 0;
      sampler_desc.BorderColor[1] = 0;
      sampler_desc.BorderColor[2] = 0;
      sampler_desc.BorderColor[3] = 0;
      sampler_desc.MinLOD = 0;
      sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

      // Create the texture sampler state
      result = device->CreateSamplerState(&sampler_desc, &sampler_state_);
      
      if (FAILED(result))
        LOGFATALF("Failed to create texture sampler state");
    };

    ~SimpleShader() {
      vertex_shader_->Release();
      pixel_shader_->Release();
    };

    void Bind(CameraObject* camera, Object* object) {
      HRESULT result;
      D3D11_MAPPED_SUBRESOURCE mapped_resource;

      result = context_->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
      if (FAILED(result))
        LOGFATALF("Failed to map matrix buffer");

      MatrixBuffer* matrices = (MatrixBuffer*)mapped_resource.pData;
      
      matrices->mat_mvp = camera->GetMVEPMatrix(object->GetWorldMatrix());

      context_->Unmap(matrix_buffer_, 0);

      context_->IASetInputLayout(layout_);

      context_->VSSetShader(vertex_shader_, nullptr, 0);
      context_->VSSetConstantBuffers(0, 1, &matrix_buffer_);

      context_->PSSetShader(pixel_shader_, nullptr, 0);

      ID3D11ShaderResourceView* texture_resource = object->GetTexture();

      context_->PSSetShaderResources(0, 1, &texture_resource);
      context_->PSSetSamplers(0, 1, &sampler_state_);
    };

    void SetTexture(ID3D11Texture2D* texture) {
    };

  private:
    ID3D11Device* device_;
    ID3D11DeviceContext* context_;

    ID3D11InputLayout* layout_;
    ID3D11Buffer* matrix_buffer_;

    ID3D11VertexShader* vertex_shader_;
    ID3D11PixelShader* pixel_shader_;

    ID3D11SamplerState* sampler_state_;
  };
};