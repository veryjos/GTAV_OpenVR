#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <vector>

#include "Object.hpp"

#include "CameraObject.hpp"
#include "SimpleShader.hpp"

namespace OVRInject
{

  /**
  Scene that renders the quads to the HMD with each eye buffer.
  */
  class RenderScene {
  public:
    RenderScene(ID3D11Device* device);
    virtual ~RenderScene();

    /**
    Initializes the scene.
    */
    virtual void Initialize();

    /**
    Releases any resources allocated by the scene.
    */
    virtual void Uninitialize();

    /**
    Renders a frame of the scene.
    */
    virtual void RenderFrame(ID3D11RenderTargetView** render_target, ID3D11DepthStencilView** depth_view, ID3D11DepthStencilState** depth_state);

    /**
    Adds an object to the scene.
    */
    virtual void AddObject(Object* object);

  private:
    /**
    Renders an object from the camera's point of view.
    */
    virtual void RenderObject(Object* object);

    CameraObject* camera_;

    SimpleShader* simple_shader_;

    ID3D11Device* device_;
    ID3D11DeviceContext* context_;

    std::vector< Object* > objects_;
  };
};