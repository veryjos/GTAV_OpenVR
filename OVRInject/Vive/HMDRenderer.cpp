#include "HMDRenderer.hpp"

using namespace OVRInject;

HMDRenderer::HMDRenderer() {
}

HMDRenderer::~HMDRenderer() {
}

void HMDRenderer::Initialize() {
	// Begin the renderer thread
	thread_handle_ = CreateThread
}

void HMDRenderer::PushFrame(ID3D11Texture2D* texture) {
	for (FrameTexture& texture : texture_buffer_) {
	}
}

void HMDRenderer::Uninitialize() {

}