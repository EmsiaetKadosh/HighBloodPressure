//
// Created by EmsiaetKadosh on 25-4-3.
//

#pragma once

#include "dxdef.h"
// #include "pack\pack.h"

#include "..\Renderer.h"
#include "font.h"

class DirectX12RenderFrame final {
	ComPtr<ID3D12Resource> coloredIndexBuffer;
	ComPtr<ID3D12Resource> coloredVertexBuffer;
	ComPtr<ID3D12Resource> textureIndexBuffer;
	ComPtr<ID3D12Resource> textureVertexBuffer;
	ComPtr<ID3D12Resource> cameraBuffer;
	ComPtr<ID3D12Resource> rtv;

};

class DirectX12Renderer final : public IRenderer {
	friend class DirectFontManager;

	ComPtr<ID3D12Device4> device = nullptr;
	ComPtr<IDXGISwapChain3> swapChain = nullptr;
	ComPtr<ID3D12Fence> fence = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV-heap
	ComPtr<ID3D12Resource> coloredBuffer[DirectX12Configs::SwapFrameCount]; // RTV-buffer
	ComPtr<ID3D12Resource> textureBuffer[DirectX12Configs::SwapFrameCount]; // RTV-buffer
	ComPtr<ID3D12Resource> textBuffer[DirectX12Configs::SwapFrameCount]; // RTV-buffer
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr; // cmd CQ
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr; // cmd CA
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr; // cmd GCL
	ComPtr<ID3D12RootSignature> rootSignature = nullptr; // RS
	ComPtr<ID3D12PipelineState> coloredPipelineState = nullptr; // PSO
	ComPtr<ID3D12PipelineState> texturePipelineState = nullptr; // PSO
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};
	HANDLE fenceEvent = nullptr;
	unsigned long long fenceValue = 0;
	unsigned int rtvDescriptorSize = 0;
	unsigned int frameIndex = 0;
	int width, height;

	inline static void getHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter) noexcept(false);
	inline static void createDevice(ComPtr<ID3D12Device4>& device, ComPtr<IDXGIFactory4>& factory) noexcept(false);
	inline static void createRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, const ComPtr<ID3D12Device4>& device) noexcept(false);
	inline static void createPipelineState(ComPtr<ID3D12PipelineState>& coloredPipelineState, ComPtr<ID3D12PipelineState>& texturePipelineState, ComPtr<ID3D12Device4>& device, ComPtr<ID3D12RootSignature>& rootSignature) noexcept(false);

public:
	DirectX12Renderer(const int width = 1920, const int height = 1080) : width(width), height(height) {
		viewport.Width = static_cast<float>(width), viewport.Height = static_cast<float>(height);
		viewport.MaxDepth = 1;
		scissorRect.right = width, scissorRect.bottom = height;
	}

	~DirectX12Renderer() override {
		fontManager->finalize();
		delete fontManager;
	}

	static void requireSucceeded(const HRESULT hr, const String& msg) noexcept(false) {
		if (FAILED(hr)) {
			const Map<QWORD, String>::const_iterator iter = directReturns.errors.find(hr);
			if (iter == directReturns.errors.end()) throw RuntimeException(L"Returns " + qwtowb16(hr, 8) + L"; " + msg);
			throw RuntimeException(iter->second + L"; " + msg);
		}
	}

	IRenderer& postInitialize() noexcept override { return initialize(), *this; }
	void initialize() noexcept(false);
	void awaitFrame() noexcept(false);
	void render() noexcept(false);
	void cleanup() noexcept(false);

	[[nodiscard]] bool checkResizing() const override;
	[[nodiscard]] unsigned int changeColorFormat(unsigned argb) const noexcept override;
	void tick() noexcept(false) override;
	void finalize(bool isRenderThread) override;
	void gameStartRender() override;
	void gameEndRender() override;
	void requireResize() override;
	void resize(int width, int height) override;
	void assertRendering() const override;
	void assertRenderThread() const override;
	void resizeStart() override;
	void resizeShow() const override;
	void resizeEnd() override;
	void renderMouseWorld() override;
	void fill(int x, int y, int w, int h, unsigned color) const override;
	void fill(const RECT* rect, unsigned color) const override;
	void fillWorld(const Vector2D& from, const Vector2D& to, unsigned color) const override;
	void fillWorld(const Vector2D& from, double blockWidth, double blockHeight, unsigned color) const override;
	void fillWorldBlock(const BlockLocation& from, unsigned color) const override;
};
