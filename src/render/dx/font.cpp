//
// Created by EmsiaetKadosh on 25-4-12.
//

#pragma once

#include "font.h"
#include "direct.h"

const DirectFont::Brush& DirectFont::getBrush(const unsigned int color) const { return manager->getBrush(color); }
int DirectFont::getWidth(const RenderableString::StringConfig& config) const noexcept { return 0; }

void DirectFont::tryCreate() const {
	const HRESULT hr = manager->dwFactory->CreateTextFormat(name.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, static_cast<float>(height), L"zh-CN", &textFormat);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to create text format for DirectFontLegacy");
}

float DirectFont::drawSingle(const RenderableString::StringConfig& config, const float x, const float y, const unsigned int defaultColor) const {
	const Brush& clr = getBrush(config.color == -1 ? defaultColor : config.color);
	HRESULT hr = 0;
	tryCreate();
	TextLayout textLayout;
	hr = manager->dwFactory->CreateTextLayout(config.text.c_str(), static_cast<unsigned int>(config.text.length()), textFormat.Get(), 0, 0, &textLayout);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to create text layout");
	DWRITE_TEXT_METRICS metrics;
	hr = textLayout->GetMetrics(&metrics);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to get metrics for text layout");
	manager->d2dContext->DrawTextLayout(D2D1::Point2F(0, 0), textLayout.Get(), clr.Get());
	return metrics.widthIncludingTrailingWhitespace;
}

void DirectFont::drawDirect(const RenderableString::StringConfig& config, float x, float y, const unsigned int defaultColor) const {
	const Brush& clr = getBrush(config.color == -1 ? defaultColor : config.color);
	HRESULT hr = 0;
	tryCreate();
	TextLayout textLayout;
	hr = manager->dwFactory->CreateTextLayout(config.text.c_str(), static_cast<unsigned int>(config.text.length()), textFormat.Get(), 0, 0, &textLayout);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to create text layout");
	manager->d2dContext->DrawTextLayout(D2D1::Point2F(0, 0), textLayout.Get(), clr.Get());
}

void DirectFont::draw(const RenderableString& text, int x, int y, const unsigned int color) const noexcept {
	manager->prepare();
	float offset = 0;
	for (const RenderableString::StringConfig& config : text.configs)
		if (config.idFont) offset = manager->get(config.idFont).drawSingle(config, offset, 0, color);
		else offset = drawSingle(config, offset, 0, color);
	manager->terminate();
}

void DirectFont::drawCenter(const RenderableString& text, int x, int y, int w, int h, unsigned color) const noexcept {}

void DirectFontManager::prepare() noexcept(false) {
	if (isRenderingOnce) throw RuntimeException(L"DirectFontManager::prepare() called while isRenderingOnce == true");
	isRenderingOnce = true;
	const DirectX12Renderer& renderer = *assert_dynamic_cast<DirectX12Renderer*>(this->renderer);
	d2dContext->BeginDraw();
	d2dContext->SetTarget(d2dTargetBitmap[renderer.frameIndex].Get());
	d2dContext->Clear(D2D1::ColorF(0, 1.0f));
}

void DirectFontManager::terminate() noexcept(false) {
	if (!isRenderingOnce) throw RuntimeException(L"DirectFontManager::terminate() called while isRenderingOnce == false");
	const HRESULT hr = d2dContext->EndDraw();
	DirectX12Renderer::requireSucceeded(hr, L"Failed to end draw");
	isRenderingOnce = false;
}

const ComPtr<ID2D1SolidColorBrush>& DirectFontManager::getBrush(const unsigned int color) const {
	if (const IterBrush iter = brushes.find(color); iter == brushes.end()) return iter->second;
	ComPtr<ID2D1SolidColorBrush> brush;
	const D2D1_COLOR_F f = {
		.r = static_cast<float>(color >> 16 & 0xff) / 255.0f,
		.g = static_cast<float>(color >> 8 & 0xff) / 255.0f,
		.b = static_cast<float>(color & 0xff) / 255.0f,
		.a = static_cast<float>(color >> 24) / 255.0f
	};
	DirectX12Renderer::requireSucceeded(d2dContext->CreateSolidColorBrush(f, &brush), L"Failed to create brush #" + qwtowb16(color, 8));
	return brushes.emplace(color, brush).first->second;
}

void DirectFontManager::renderStart() {
	const DirectX12Renderer& renderer = *assert_dynamic_cast<DirectX12Renderer*>(this->renderer);
	// 1. 获取当前后备缓冲区
	// 2. 重新绑定共享目标（如果交换链大小变化需重建，此处省略）
	// 注意：若交换链重建（如窗口大小改变），需释放旧资源并重新调用 CreateSharedRenderTarget
	// 3. 释放 D3D11 对资源的占用（确保 D3D12 可以修改）
	deviceOn12->AcquireWrappedResources(textBufferShadow[renderer.frameIndex].GetAddressOf(), 1);
	// 4. 转换资源状态（D3D12 侧）
	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderer.textBuffer[renderer.frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	renderer.commandList->ResourceBarrier(1, &barrier);
}

void DirectFontManager::renderEnd() {
	const DirectX12Renderer& renderer = *assert_dynamic_cast<DirectX12Renderer*>(this->renderer);
	// 1. 释放 D3D11 对资源的占用
	deviceOn12->ReleaseWrappedResources(textBufferShadow[renderer.frameIndex].GetAddressOf(), 1);
	context11->Flush();
	// 2. 转换资源状态回 PRESENT
	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderer.textBuffer[renderer.frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	renderer.commandList->ResourceBarrier(1, &barrier);
}

DirectFontManager::DirectFontManager(DirectX12Renderer* const renderer): IFontManager(renderer) {
	HRESULT hr = 0;
	// 1. 创建 D3D11On12 设备
	hr = D3D11On12CreateDevice(renderer->device.Get(), D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, nullptr, 0, 0, &device11, &context11, nullptr);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to create D3D 11 on 12 device");
	// 2. 查询 D3D11On12 接口
	hr = device11.As(&deviceOn12);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to get D3D 11 on 12 device");
	// 3. 创建 D2D 工厂
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory7), nullptr, &d2dFactory);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to create D2D1 factory");
	// 4. 创建 DirectWrite 工厂
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory7), &dwFactory);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to create DirectWrite Factory");
	// 5. 创建 D2D 设备（关联 D3D11）
	ComPtr<IDXGIDevice> dxgiDevice;
	hr = device11.As(&dxgiDevice);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to get DXGI device from device11");
	hr = d2dFactory->CreateDevice(dxgiDevice.Get(), &d2dDevice);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to create D2D device");
	// 6. 创建 D2D 设备上下文
	hr = d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContext);
	DirectX12Renderer::requireSucceeded(hr, L"Failed to create D2D context");
	// 7. 创建文本格式

	// 1. 获取初始后备缓冲区（假设交换链已创建）
	constexpr D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
	const D2D1_BITMAP_PROPERTIES1 bitmapProps = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	for (unsigned int i = 0; i < DirectX12Configs::SwapFrameCount; ++i) {
		// 2. 包装 D3D12 资源到 D3D11
		hr = deviceOn12->CreateWrappedResource(renderer->textBuffer[i].Get(), &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&textBufferShadow[i]));
		DirectX12Renderer::requireSucceeded(hr, L"Failed to create prepare textBufferShadow[" + std::to_wstring(i) + L"]");
		// 3. 创建 D2D 位图绑定到共享纹理
		ComPtr<IDXGISurface> dxgiSurface;
		hr = textBufferShadow[i].As(&dxgiSurface);
		DirectX12Renderer::requireSucceeded(hr, L"Failed to get dxgi surface[" + std::to_wstring(i) + L"]");
		hr = d2dContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProps, &d2dTargetBitmap[i]);
		DirectX12Renderer::requireSucceeded(hr, L"Failed to create bitmap[" + std::to_wstring(i) + L"]");
	}
}

void DirectFontManager::finalize() {}
void DirectFontManager::resize(int width, int height) {}

DirectFont& DirectFontManager::newFont(const String& name, const double heightModifier, const double yOffset, const bool adaptAllSize, const long escapement, const long orientation) {
	++assigned;
	return fonts.emplace(assigned, std::move(DirectFont(this, assigned, name, heightModifier, yOffset, adaptAllSize, escapement, orientation))).first->second;
}

DirectFont& DirectFontManager::newFont(String&& name, const double heightModifier, const double yOffset, const bool adaptAllSize, const long escapement, const long orientation) {
	++assigned;
	return fonts.emplace(assigned, std::move(DirectFont(this, assigned, std::move(name), heightModifier, yOffset, adaptAllSize, escapement, orientation))).first->second;
}

DirectFont& DirectFontManager::getDefault() const noexcept { return *defaultFont; }
DirectFont& DirectFontManager::get(FontID id) const noexcept { return *defaultFont; }
