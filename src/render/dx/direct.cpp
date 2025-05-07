//
// Created by EmsiaetKadosh on 25-4-4.
//

#include "direct.h"

bool DirectX12Renderer::checkResizing() const { return false; }
unsigned int DirectX12Renderer::changeColorFormat(const unsigned int argb) const noexcept { return argb; }
void DirectX12Renderer::tick() noexcept(false) {}
void DirectX12Renderer::finalize(bool isRenderThread) {}
void DirectX12Renderer::gameStartRender() {}
void DirectX12Renderer::gameEndRender() {}
void DirectX12Renderer::requireResize() {}
void DirectX12Renderer::resize(int width, int height) {}
void DirectX12Renderer::assertRendering() const {}
void DirectX12Renderer::assertRenderThread() const {}
void DirectX12Renderer::resizeStart() {}
void DirectX12Renderer::resizeShow() const {}
void DirectX12Renderer::resizeEnd() {}
void DirectX12Renderer::renderMouseWorld() {}
void DirectX12Renderer::fill(int x, int y, int w, int h, unsigned color) const {}
void DirectX12Renderer::fill(const RECT* rect, unsigned color) const {}
void DirectX12Renderer::fillWorld(const Vector2D& from, const Vector2D& to, unsigned color) const {}
void DirectX12Renderer::fillWorld(const Vector2D& from, double blockWidth, double blockHeight, unsigned color) const {}
void DirectX12Renderer::fillWorldBlock(const BlockLocation& from, unsigned color) const {}
