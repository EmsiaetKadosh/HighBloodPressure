//
// Created by EmsiaetKadosh on 25-4-4.
//

#include "..\..\def.h"
#include "..\..\hbp.h"
#include "..\..\utils\exception.h"
#include "..\..\utils\Chars.h"

#include "direct.h"
#include "hlsl.h"

inline void DirectX12Renderer::requireSucceeded(const HRESULT hr, const String& msg) noexcept(false) {
	if (FAILED(hr)) {
		const Map<QWORD, String>::iterator iter = directReturns.errors.find(hr);
		if (iter == directReturns.errors.end()) throw RuntimeException(L"Returns " + qwtowb16(hr, 8) + L"; " + msg);
		throw RuntimeException(iter->second + L"; " + msg);
	}
}

/**
 * @brief 辅助函数，获取硬件适配器
 * @param pFactory D3D工厂
 * @param ppAdapter [out] 适配器
 * @param requestHighPerformanceAdapter 是否需求高性能，一般都传入true
 * @note 一般只在
 * @code createDevice @endcode
 * 中调用
 */
inline void DirectX12Renderer::getHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter, const bool requestHighPerformanceAdapter) noexcept(false) {
	*ppAdapter = nullptr;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory6> factory;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory)))) // DXGI 1.6+ 支持按性能排序枚举适配器
		for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(adapterIndex, requestHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))); ++adapterIndex) {
			DXGI_ADAPTER_DESC1 desc;
			discard_return(adapter->GetDesc1(&desc));
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, _uuidof(ID3D12Device), nullptr))) break;
		}
	if (adapter.Get() == nullptr)
		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex) {
			DXGI_ADAPTER_DESC1 desc;
			discard_return(adapter->GetDesc1(&desc));
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, _uuidof(ID3D12Device), nullptr))) break;
		}
	*ppAdapter = adapter.Detach();
}

/**
 * @brief 辅助函数，创建一个device。
 * @param device [out] 创建的device
 * @param factory [out] 创建的factory，用于复用、避免二次创建
 * @note 创建失败时直接throw，如果没有throw，那么确保device中含有一个可用的设备
 */
inline void DirectX12Renderer::createDevice(ComPtr<ID3D12Device4>& device, ComPtr<IDXGIFactory4>& factory) noexcept(false) {
	HRESULT hr = 0;
	unsigned factoryFlag = 0;
	// 1. 调试层最先设置；__CARLBEKS_DEBUG__宏控制是否启用
#if defined __CARLBEKS_DEBUG__
	{
		ComPtr<ID3D12Debug> debugController;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
		if (SUCCEEDED(hr)) {
			debugController->EnableDebugLayer();
			factoryFlag = DXGI_CREATE_FACTORY_DEBUG;
			ComPtr<ID3D12Debug1> debugController1;
			hr = debugController.As(&debugController1);
			if (SUCCEEDED(hr)) debugController1->SetEnableGPUBasedValidation(true);
		}
		if (FAILED(hr)) {
			if (const Map<QWORD, String>::iterator iter = directReturns.errors.find(hr); iter == directReturns.errors.end()) Logger.error(L"Returns " + qwtowb16(hr, 8) + L"; Failed to create debugController");
			else Logger.error(iter->second + L"; Failed to create debugController");
		}
	}
#endif // !defined(__CARLBEKS_DEBUG__)
	// 2. 创建工厂
	hr = CreateDXGIFactory2(factoryFlag, IID_PPV_ARGS(&factory));
	requireSucceeded(hr, L"Failed to create dxgiFactory");
	// 3. 枚举并获取最佳适配器
	ComPtr<IDXGIAdapter1> dxgiAdapter;
	getHardwareAdapter(factory.Get(), &dxgiAdapter, true);
	// 4. D3D12设备
	hr = D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_2 /* 编写代码时的最高级别 */, IID_PPV_ARGS(&device));
	requireSucceeded(hr, L"Failed to create D3D12 device");
	// 5. 检查功能支持情况
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 features = {};
	if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features, sizeof(features))))
		// 检查光线追踪支持等新特性
		if (features.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) Logger.warn(L"Raytracing not supported!");
}

void DirectX12Renderer::createRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, const ComPtr<ID3D12Device4>& device) noexcept(false) {
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	slotRootParameter[0].InitAsConstantBufferView(0); // CBV寄存器0
	const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{ 1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT };
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	const HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
	if (errorBlob != nullptr) Logger.error(atow(static_cast<char*>(errorBlob->GetBufferPointer())));
	requireSucceeded(hr, L"Failed to serialize D3D12 root signature");
	requireSucceeded(device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), L"Failed to create D3D12 root signature");
}

void DirectX12Renderer::createPipelineState(ComPtr<ID3D12PipelineState>& coloredPipelineState, ComPtr<ID3D12PipelineState>& texturePipelineState, ComPtr<ID3D12Device4>& device, ComPtr<ID3D12RootSignature>& rootSignature) noexcept(false) {
	// TODO(EmsiaetKadosh): 编译着色器
	// 定义顶点输入布局
	D3D12_INPUT_ELEMENT_DESC coloredLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	D3D12_INPUT_ELEMENT_DESC textureLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R16G16B16A16_SNORM, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	// 填充PSO描述结构
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { coloredLayout, 1 };
	psoDesc.pRootSignature = rootSignature.Get();
	discard_return(psoDesc.VS);
	discard_return(psoDesc.PS);
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	requireSucceeded(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&coloredPipelineState)), L"Failed to create graphics pipeline");
	// 第二结构
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc2 = {};
	psoDesc2.InputLayout = { textureLayout, 1 };
	psoDesc2.pRootSignature = rootSignature.Get();
	discard_return(psoDesc2.VS);
	discard_return(psoDesc2.PS);
	psoDesc2.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc2.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc2.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc2.SampleMask = UINT_MAX;
	psoDesc2.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc2.NumRenderTargets = 1;
	psoDesc2.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc2.SampleDesc.Count = 1;
	requireSucceeded(device->CreateGraphicsPipelineState(&psoDesc2, IID_PPV_ARGS(&texturePipelineState)), L"Failed to create graphics pipeline");
}

void DirectX12Renderer::initialize() noexcept(false) {
	HRESULT hr = 0;
	// 1. 创建设备，顺带创建工厂
	ComPtr<IDXGIFactory4> factory;
	createDevice(device, factory); // 失败则抛，以下代码执行时保证创建成功
	// 2. 创建CQ
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
		requireSucceeded(hr, L"Failed to create command queue");
	}
	// 3. 创建SwapChain
	{
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.BufferCount = 2;
		desc.Width = width;
		desc.Height = height;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.SampleDesc.Count = 1;
		ComPtr<IDXGISwapChain1> chain;
		hr = factory->CreateSwapChainForHwnd(commandQueue.Get(), MainWindowHandle, &desc, nullptr, nullptr, &chain);
		requireSucceeded(hr, L"Failed to create swap chain");
		hr = chain.As(&swapChain);
		requireSucceeded(hr, L"Failed to cast swap chain");
		frameIndex = swapChain->GetCurrentBackBufferIndex();
	}
	// 4. 创建RTV描述符堆
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 2;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap));
		requireSucceeded(hr, L"Failed to create rtv heap");
		rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	// 5. 创建帧缓冲渲染RTV
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < RenderTargetCount; ++n) {
			hr = swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]));
			requireSucceeded(hr, L"Failed to get render target; n = " + std::to_wstring(n));
			device->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, rtvDescriptorSize);
		}
	}
	// 6. 创建命令分配器和命令列表
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
		requireSucceeded(hr, L"Failed to create command allocator");
		hr = device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList));
		requireSucceeded(hr, L"Failed to create command list");
	}
	// 7. 创建同步对象（Fence）
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		requireSucceeded(hr, L"Failed to create fence");
		fenceValue = 1;
		fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent == nullptr) requireSucceeded(HRESULT_FROM_WIN32(GetLastError()), L"Failed to create fence event");
	}
	// 以下在示例的LoadAssets
	// 1. 根签名
		createRootSignature(rootSignature, device);
	// 2. PSO
	createPipelineState(coloredPipelineState, texturePipelineState, device, rootSignature);
}

void DirectX12Renderer::cleanup() noexcept(false) {
	// 等待GPU完成所有工作
	awaitFrame();
	CloseHandle(fenceEvent);
	// 所有ComPtr对象会自动释放
}

void DirectX12Renderer::awaitFrame() noexcept(false) {
	// 信号并等待
	const UINT64 fence = fenceValue;
	discard_return(commandQueue->Signal(this->fence.Get(), fence));
	++this->fenceValue;
	if (this->fence->GetCompletedValue() < fence) {
		discard_return(this->fence->SetEventOnCompletion(fence, this->fenceEvent));
		WaitForSingleObject(this->fenceEvent, INFINITE);
	}
	frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void DirectX12Renderer::render() noexcept(false) {
	awaitFrame();
	// 1. 重置命令列表和分配器
	requireSucceeded(commandAllocator->Reset(), L"Failed to reset command allocator");
	requireSucceeded(commandList->Reset(commandAllocator.Get(), coloredPipelineState.Get()), L"Failed to reset command list");
	// 2. 设置视口和裁剪区域
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	// 3. 设置渲染目标
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	commandList->OMSetRenderTargets(1, &rtvHandle, 0, nullptr);
	// 4. 清除渲染目标
	constexpr float clearColor[] = { 0.2f, 0.4f, 0.6f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	// 5.1. 设置图形管线状态
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(coloredPipelineState.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW view = {
		.BufferLocation = renderTargets[frameIndex]->GetGPUVirtualAddress(),
		.SizeInBytes = 0,
		.StrideInBytes = sizeof(ColoredVertex)
	};
	commandList->IASetVertexBuffers(0, 1, &view);
	// 6.1. 绘制调用
	commandList->DrawInstanced(0 /* TODO(EmsiaetKadosh): 数量 */, 1, 0, 0); // 示例：绘制一个三角形
	// 5.2. 设置图形管线状态
	commandList->SetPipelineState(texturePipelineState.Get()    );
	// 6.2. 绘制调用
	commandList->DrawInstanced(0 /* TODO(EmsiaetKadosh): 数量 */, 1, 0 /* 数量 */, 0); // 示例：绘制一个三角形
	// 7. 提交命令列表
	requireSucceeded(commandList->Close(), L"Failed to close command list");
	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, cmdLists);
	// 8. 呈现交换链
	requireSucceeded(swapChain->Present(1, 0), L"Failed to present swap chain");
}
