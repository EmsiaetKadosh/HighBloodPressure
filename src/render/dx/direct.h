//
// Created by EmsiaetKadosh on 25-4-3.
//

#pragma once

#include "..\..\def.h"

// #include "pack\pack.h"

#define discard_return(exp) (exp)

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class [[carlbeks::defineat("renderer.h")]] Renderer;

struct TextureVertex {
	DirectX::XMFLOAT3 position; // 位置 (R32G32B32_FLOAT)
	DirectX::PackedVector::XMCOLOR color; // 颜色 (R8G8B8A8_UNORM，需手动归一化)
	DirectX::XMFLOAT2 uv; // UV坐标 (R32G32_FLOAT，贴图时使用)
	DirectX::PackedVector::XMSHORT4 normal; // 法线 (R16G16B16A16_SNORM，光照时使用)
};

struct ColoredVertex {
	DirectX::XMFLOAT3 position; // 位置 (R32G32B32_FLOAT)
	DirectX::PackedVector::XMCOLOR color; // 颜色 (R8G8B8A8_UNORM，需手动归一化)
};

struct ReturnCode {
	inline static Map<QWORD, String> errors;

	ReturnCode() {
		errors.emplace(D3D12_ERROR_ADAPTER_NOT_FOUND, L"D3D12_ERROR_ADAPTER_NOT_FOUND: 指定的缓存PSO是在不同的适配器上创建的，不能在当前适配器上重复使用。");
		errors.emplace(D3D12_ERROR_DRIVER_VERSION_MISMATCH, L"D3D12_ERROR_DRIVER_VERSION_MISMATCH: 指定的缓存PSO是在不同的驱动程序版本上创建的，不能在当前适配器上重复使用。");
		errors.emplace(DXGI_ERROR_INVALID_CALL, L"DXGI_ERROR_INVALID_CALL: 方法调用无效。例如，方法的参数可能不是有效的指针。");
		errors.emplace(DXGI_ERROR_WAS_STILL_DRAWING, L"DXGI_ERROR_WAS_STILL_DRAWING: 将信息传输到此图面或从此图面传输信息的上一个blit不完整。");
		errors.emplace(E_FAIL, L"E_FAIL: 尝试创建启用了调试层且未安装该层的设备。");
		errors.emplace(E_INVALIDARG, L"E_INVALIDARG: 将无效参数传递给返回函数。");
		errors.emplace(E_OUTOFMEMORY, L"E_OUTOFMEMORY: Direct3D无法分配足够的内存来完成调用。");
		errors.emplace(E_NOTIMPL, L"E_NOTIMPL: 方法调用不是使用传递的参数组合实现的。");
		errors.emplace(S_FALSE, L"S_FALSE： 备用成功值，指示成功但非标准完成（精确含义取决于上下文）。");
		errors.emplace(S_OK, L"S_OK: 未发生错误。");
		errors.emplace(DXGI_ERROR_ACCESS_DENIED, L"DXGI_ERROR_ACCESS_DENIED: 您尝试使用没有所需访问权限的资源。 此错误通常是在写入具有只读访问权限的共享资源时导致的。");
		errors.emplace(DXGI_ERROR_ACCESS_LOST, L"DXGI_ERROR_ACCESS_LOST: 桌面重复接口无效。当桌面上显示不同类型的图像时，桌面重复界面通常会失效。");
		errors.emplace(DXGI_ERROR_ALREADY_EXISTS, L"DXGI_ERROR_ALREADY_EXISTS: 所需的元素已存在。如果不是第一次调用函数，则DXGIDeclareAdapterRemovalSupport会返回此函数。");
		errors.emplace(DXGI_ERROR_CANNOT_PROTECT_CONTENT, L"DXGI_ERROR_CANNOT_PROTECT_CONTENT: DXGI无法在交换链上提供内容保护。此错误通常是由较旧的驱动程序引起的，或者当你使用与内容保护不兼容的交换链时。");
		errors.emplace(DXGI_ERROR_DEVICE_HUNG, L"DXGI_ERROR_DEVICE_HUNG: 由于应用程序发送的命令格式不正确，应用程序的设备出现故障。 这是一个设计时问题，应进行调查和修复。");
		errors.emplace(DXGI_ERROR_DEVICE_REMOVED, L"DXGI_ERROR_DEVICE_REMOVED: 视频卡已实际从系统中删除，或者视频卡的驱动程序升级。 应用程序应销毁并重新创建设备。有关调试问题的帮助，请调用ID3D10Device::GetDeviceRemovedReason。");
		errors.emplace(DXGI_ERROR_DEVICE_RESET, L"DXGI_ERROR_DEVICE_RESET: 由于命令格式不正确，设备失败。这是一个运行时问题；应用程序应销毁并重新创建设备。");
		errors.emplace(DXGI_ERROR_DRIVER_INTERNAL_ERROR, L"DXGI_ERROR_DRIVER_INTERNAL_ERROR: 驱动程序遇到问题，并已进入设备删除状态。");
		errors.emplace(DXGI_ERROR_FRAME_STATISTICS_DISJOINT, L"DXGI_ERROR_FRAME_STATISTICS_DISJOINT: 例如，某个事件（电源周期）中断了当前统计信息的收集。");
		errors.emplace(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE, L"DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: 应用程序尝试获取输出的独占所有权，但失败，因为应用程序中的一些其他应用程序（或设备）已获取所有权。");
		errors.emplace(DXGI_ERROR_MORE_DATA, L"DXGI_ERROR_MORE_DATA: 应用程序提供的缓冲区不够大，无法容纳请求的数据。");
		errors.emplace(DXGI_ERROR_NAME_ALREADY_EXISTS, L"DXGI_ERROR_NAME_ALREADY_EXISTS: 在调用IDXGIResource1::CreateSharedHandle时提供的资源名称已与某些其他资源相关联。");
		errors.emplace(DXGI_ERROR_NONEXCLUSIVE, L"DXGI_ERROR_NONEXCLUSIVE: 全局计数器资源正在使用中，Direct3D设备当前无法使用计数器资源。");
		errors.emplace(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE, L"DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: 资源或请求当前不可用，但以后可能会变得可用。");
		errors.emplace(DXGI_ERROR_NOT_FOUND, L"DXGI_ERROR_NOT_FOUND: 调用IDXGIObject::GetPrivateData 时，不会将传入的 GUID识别为以前传递给IDXGIObject::SetPrivateData或IDXGIObject::SetPrivateDataInterface的GUID。调用IDXGIFactory::EnumAdapters或IDXGIAdapter::EnumOutputs时，枚举的序号已超过范围。");
		errors.emplace(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED, L"DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED预留");
		errors.emplace(DXGI_ERROR_REMOTE_OUTOFMEMORY, L"DXGI_ERROR_REMOTE_OUTOFMEMORY: DXGI_ERROR_REMOTE_OUTOFMEMORY预留");
		errors.emplace(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE, L"DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: 交换链内容受限（监视器）的DXGI输出现在已断开连接或更改。");
		errors.emplace(DXGI_ERROR_SDK_COMPONENT_MISSING, L"DXGI_ERROR_SDK_COMPONENT_MISSING: 操作依赖于缺少或不匹配的SDK组件。");
		errors.emplace(DXGI_ERROR_SESSION_DISCONNECTED, L"DXGI_ERROR_SESSION_DISCONNECTED: 远程桌面服务会话当前已断开连接。");
		errors.emplace(DXGI_ERROR_UNSUPPORTED, L"DXGI_ERROR_UNSUPPORTED: 设备或驱动程序不支持请求的功能。");
		errors.emplace(DXGI_ERROR_WAIT_TIMEOUT, L"DXGI_ERROR_WAIT_TIMEOUT: 在下一个桌面帧可用之前经过的超时间隔。");
		errors.emplace(DXGI_ERROR_WAS_STILL_DRAWING, L"DXGI_ERROR_WAS_STILL_DRAWING: GPU在调用执行操作时处于繁忙状态，并且未执行或计划操作。");
	}
} inline directReturns;

class DirectX12Renderer {
	static constexpr QWORD RenderTargetCount = 2;

	ComPtr<ID3D12Device4> device = nullptr;
	ComPtr<IDXGISwapChain3> swapChain = nullptr;
	ComPtr<ID3D12Fence> fence = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV-heap
	ComPtr<ID3D12Resource> renderTargets[RenderTargetCount]; // RTV-buffer
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

	inline static void requireSucceeded(HRESULT hr, const String& msg) noexcept(false);

	void initialize() noexcept(false);
	void awaitFrame() noexcept(false);
	void render() noexcept(false);
	void cleanup() noexcept(false);
};
