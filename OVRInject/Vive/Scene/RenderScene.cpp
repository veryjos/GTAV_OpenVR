#include "targetver.h"

#include "RenderScene.hpp"

using namespace OVRInject;

RenderScene::RenderScene(ID3D11Device* device) {
  device_ = device;

  device_->GetImmediateContext(&context_);
}

RenderScene::~RenderScene() {

  Uninitialize();
}

void RenderScene::Initialize() {
  camera_ = new CameraObject(device_);

  simple_shader_ = new SimpleShader(device_);
}

void RenderScene::Uninitialize() {
  delete camera_;

  delete simple_shader_;

  for (auto o : objects_)
    delete o;

  objects_.clear();
}

void RenderScene::RenderFrame(ID3D11RenderTargetView** render_target, ID3D11DepthStencilView** depth_view, ID3D11DepthStencilState** depth_state) {
  const float clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };

  context_->OMSetDepthStencilState(*depth_state, 1);
  context_->OMSetRenderTargets(1, render_target, *depth_view);

  context_->ClearRenderTargetView(*render_target, clear_color);
  context_->ClearDepthStencilView(*depth_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

  for (auto o : objects_)
    RenderObject(o);
}

void RenderScene::AddObject(Object* object) {
  objects_.push_back(object);
}

void RenderScene::RenderObject(Object* object) {
  unsigned int stride = sizeof(ObjectVertex);
  unsigned int offset = 0;

  context_->IASetVertexBuffers(0, 1, &object->vertex_buffer_, &stride, &offset);
  context_->IASetIndexBuffer(object->index_buffer_, DXGI_FORMAT_R32_UINT, 0);

  context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  simple_shader_->Bind(camera_, object);

  context_->DrawIndexed(object->index_count_, 0, 0);
}

CameraObject* RenderScene::GetCamera() {
  return camera_;
}