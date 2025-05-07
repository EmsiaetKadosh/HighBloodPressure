//
// Created by EmsiaetKadosh on 25-4-12.
//

#pragma once

#include "..\..\def.h"
#include "..\..\utils\math.h"
#include "..\..\utils\exception.h"

#include "..\..\..\..\DirectX-lib\include\directx\d3d12.h"
#include "..\..\..\..\DirectX-lib\include\directx\d3dx12.h"
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <D3DCompiler.h>
#include <DirectXPackedVector.h>
#if defined __CARLBEKS_DEBUG__ || true
#include <dxgidebug.h>
#endif
#include <wrl.h>

#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite_3.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "D3DCompiler.lib")

#define discard_return(exp) (exp)

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct DirectX12Configs {
	static constexpr QWORD SwapFrameCount = 2;
};

struct CameraView {
	DirectX::XMFLOAT4X4 conMatrix;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

struct TextureVertex {
	float position[3]; // 位置 (R32G32B32_FLOAT)
	unsigned int color; // 颜色 (R8G8B8A8_UNORM，需手动归一化)
	float uv[2]; // UV坐标 (R32G32_FLOAT，贴图时使用)
	short normal[4]; // 法线 (R16G16B16A16_SNORM，光照时使用)

	static constexpr D3D12_INPUT_ELEMENT_DESC LAYOUT[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32_UINT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R16G16B16A16_SNORM, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	static constexpr size_t LAYOUT_SIZE = 4;
};

struct ColoredVertex {
	float position[3]; // 位置 (R32G32B32_FLOAT)
	unsigned int color; // 颜色 (R8G8B8A8_UNORM，需手动归一化)

	static constexpr D3D12_INPUT_ELEMENT_DESC LAYOUT[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32_UINT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	static constexpr size_t LAYOUT_SIZE = 2;
};

class IndexSet {
	friend class DirectX12RenderFrame;
	friend class DirectFrame;
	unsigned int count;
	bool own;
	unsigned int* indices;

public:
	IndexSet(const unsigned int count) noexcept(false) : count(count), own(true), indices(new unsigned int[count]) {}
	IndexSet(const IndexSet& other) noexcept : count(other.count), own(false), indices(other.indices) {}
	IndexSet(IndexSet&& other) noexcept : count(other.count), own(other.own), indices(other.indices) { other.count = 0, other.own = false, other.indices = nullptr; }
	~IndexSet() { if (own) delete[] indices; }

	[[nodiscard]] unsigned int& at(const unsigned int offset) noexcept(false) {
		if (offset >= count) throw ArrayIndexOutOfBoundException(L"offset >= count");
		return indices[offset];
	}
};

class TextureVertexSet {
	friend class DirectX12RenderFrame;
	friend class DirectFrame;
	unsigned int count;
	bool own;
	TextureVertex* vertex;

public:
	TextureVertexSet(const unsigned int count) noexcept(false) : count(count), own(true), vertex(new TextureVertex[count]) {}
	TextureVertexSet(const TextureVertexSet& other) noexcept : count(other.count), own(false), vertex(other.vertex) {};
	TextureVertexSet(TextureVertexSet&& other) noexcept : count(other.count), own(other.own), vertex(other.vertex) { other.count = 0, other.own = false, other.vertex = nullptr; }
	~TextureVertexSet() { if (own) delete[] vertex; }

	[[nodiscard]] TextureVertex& at(const unsigned int offset) noexcept(false) {
		if (offset >= count) throw ArrayIndexOutOfBoundException(L"offset >= count");
		return vertex[offset];
	}
};

class ColoredVertexSet {
	friend class DirectX12RenderFrame;
	friend class DirectFrame;
	unsigned int count;
	bool own;
	ColoredVertex* vertex;

public:
	ColoredVertexSet(const unsigned int count) noexcept(false) : count(count), own(true), vertex(new ColoredVertex[count]) {}
	ColoredVertexSet(const ColoredVertexSet& other) noexcept : count(other.count), own(false), vertex(other.vertex) {};
	ColoredVertexSet(ColoredVertexSet&& other) noexcept : count(other.count), own(other.own), vertex(other.vertex) { other.count = 0, other.own = false, other.vertex = nullptr; }
	~ColoredVertexSet() { if (own) delete[] vertex; }

	[[nodiscard]] ColoredVertex& at(const unsigned int offset) noexcept(false) {
		if (offset >= count) throw ArrayIndexOutOfBoundException(L"offset >= count");
		return vertex[offset];
	}
};

class ReturnCode {
	static const ReturnCode instance;

public:
	Map<QWORD, String> errors;

	static const ReturnCode& getInstance() noexcept { return instance; }

private:
	ReturnCode() {
		errors.emplace(D3D12_ERROR_ADAPTER_NOT_FOUND, L"D3D12_ERROR_ADAPTER_NOT_FOUND: 指定的缓存PSO是在不同的适配器上创建的，不能在当前适配器上重复使用");
		errors.emplace(D3D12_ERROR_DRIVER_VERSION_MISMATCH, L"D3D12_ERROR_DRIVER_VERSION_MISMATCH: 指定的缓存PSO是在不同的驱动程序版本上创建的，不能在当前适配器上重复使用");
		errors.emplace(DXGI_ERROR_INVALID_CALL, L"DXGI_ERROR_INVALID_CALL: 方法调用无效。例如，方法的参数可能不是有效的指针");
		errors.emplace(DXGI_ERROR_WAS_STILL_DRAWING, L"DXGI_ERROR_WAS_STILL_DRAWING: 将信息传输到此图面或从此图面传输信息的上一个blit不完整");
		errors.emplace(E_FAIL, L"E_FAIL: 尝试创建启用了调试层且未安装该层的设备");
		errors.emplace(E_INVALIDARG, L"E_INVALIDARG: 将无效参数传递给返回函数");
		errors.emplace(E_OUTOFMEMORY, L"E_OUTOFMEMORY: Direct3D无法分配足够的内存来完成调用");
		errors.emplace(E_NOTIMPL, L"E_NOTIMPL: 方法调用不是使用传递的参数组合实现的");
		errors.emplace(S_FALSE, L"S_FALSE： 备用成功值，指示成功但非标准完成（精确含义取决于上下文）");
		errors.emplace(S_OK, L"S_OK: 未发生错误");
		errors.emplace(DXGI_ERROR_ACCESS_DENIED, L"DXGI_ERROR_ACCESS_DENIED: 您尝试使用没有所需访问权限的资源。 此错误通常是在写入具有只读访问权限的共享资源时导致的");
		errors.emplace(DXGI_ERROR_ACCESS_LOST, L"DXGI_ERROR_ACCESS_LOST: 桌面重复接口无效。当桌面上显示不同类型的图像时，桌面重复界面通常会失效");
		errors.emplace(DXGI_ERROR_ALREADY_EXISTS, L"DXGI_ERROR_ALREADY_EXISTS: 所需的元素已存在。如果不是第一次调用函数，则DXGIDeclareAdapterRemovalSupport会返回此函数");
		errors.emplace(DXGI_ERROR_CANNOT_PROTECT_CONTENT, L"DXGI_ERROR_CANNOT_PROTECT_CONTENT: DXGI无法在交换链上提供内容保护。此错误通常是由较旧的驱动程序引起的，或者当你使用与内容保护不兼容的交换链时");
		errors.emplace(DXGI_ERROR_DEVICE_HUNG, L"DXGI_ERROR_DEVICE_HUNG: 由于应用程序发送的命令格式不正确，应用程序的设备出现故障。 这是一个设计时问题，应进行调查和修复");
		errors.emplace(DXGI_ERROR_DEVICE_REMOVED, L"DXGI_ERROR_DEVICE_REMOVED: 视频卡已实际从系统中删除，或者视频卡的驱动程序升级。 应用程序应销毁并重新创建设备。有关调试问题的帮助，请调用ID3D10Device::GetDeviceRemovedReason");
		errors.emplace(DXGI_ERROR_DEVICE_RESET, L"DXGI_ERROR_DEVICE_RESET: 由于命令格式不正确，设备失败。这是一个运行时问题；应用程序应销毁并重新创建设备");
		errors.emplace(DXGI_ERROR_DRIVER_INTERNAL_ERROR, L"DXGI_ERROR_DRIVER_INTERNAL_ERROR: 驱动程序遇到问题，并已进入设备删除状态");
		errors.emplace(DXGI_ERROR_FRAME_STATISTICS_DISJOINT, L"DXGI_ERROR_FRAME_STATISTICS_DISJOINT: 例如，某个事件（电源周期）中断了当前统计信息的收集");
		errors.emplace(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE, L"DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: 应用程序尝试获取输出的独占所有权，但失败，因为应用程序中的一些其他应用程序（或设备）已获取所有权");
		errors.emplace(DXGI_ERROR_MORE_DATA, L"DXGI_ERROR_MORE_DATA: 应用程序提供的缓冲区不够大，无法容纳请求的数据");
		errors.emplace(DXGI_ERROR_NAME_ALREADY_EXISTS, L"DXGI_ERROR_NAME_ALREADY_EXISTS: 在调用IDXGIResource1::CreateSharedHandle时提供的资源名称已与某些其他资源相关联");
		errors.emplace(DXGI_ERROR_NONEXCLUSIVE, L"DXGI_ERROR_NONEXCLUSIVE: 全局计数器资源正在使用中，Direct3D设备当前无法使用计数器资源");
		errors.emplace(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE, L"DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: 资源或请求当前不可用，但以后可能会变得可用");
		errors.emplace(DXGI_ERROR_NOT_FOUND, L"DXGI_ERROR_NOT_FOUND: 调用IDXGIObject::GetPrivateData 时，不会将传入的 GUID识别为以前传递给IDXGIObject::SetPrivateData或IDXGIObject::SetPrivateDataInterface的GUID。调用IDXGIFactory::EnumAdapters或IDXGIAdapter::EnumOutputs时，枚举的序号已超过范围");
		errors.emplace(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED, L"DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED预留");
		errors.emplace(DXGI_ERROR_REMOTE_OUTOFMEMORY, L"DXGI_ERROR_REMOTE_OUTOFMEMORY: DXGI_ERROR_REMOTE_OUTOFMEMORY预留");
		errors.emplace(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE, L"DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: 交换链内容受限（监视器）的DXGI输出现在已断开连接或更改");
		errors.emplace(DXGI_ERROR_SDK_COMPONENT_MISSING, L"DXGI_ERROR_SDK_COMPONENT_MISSING: 操作依赖于缺少或不匹配的SDK组件");
		errors.emplace(DXGI_ERROR_SESSION_DISCONNECTED, L"DXGI_ERROR_SESSION_DISCONNECTED: 远程桌面服务会话当前已断开连接");
		errors.emplace(DXGI_ERROR_UNSUPPORTED, L"DXGI_ERROR_UNSUPPORTED: 设备或驱动程序不支持请求的功能");
		errors.emplace(DXGI_ERROR_WAIT_TIMEOUT, L"DXGI_ERROR_WAIT_TIMEOUT: 在下一个桌面帧可用之前经过的超时间隔");
		errors.emplace(DXGI_ERROR_WAS_STILL_DRAWING, L"DXGI_ERROR_WAS_STILL_DRAWING: GPU在调用执行操作时处于繁忙状态，并且未执行或计划操作");
	}
} inline const& directReturns = ReturnCode::getInstance();

inline const ReturnCode ReturnCode::instance = ReturnCode();

inline void requireSucceeded(HRESULT hr, const String& msg) noexcept(false);

template<typename T> requires std::is_base_of_v<IUnknown, T>
void checkReference(T* object) noexcept {
	object->AddRef();
	Logger.error(atow(typeid(T).name()) + L" refCount: " + std::to_wstring(object->Release()));
}

inline const char* Shader = R"shader(
	cbuffer ObjectConstants : register(b0) {
		float4x4 g_matrix;  // 预计算的 View × Proj 矩阵
		float4x4 view; // 视野
		float4x4 proj; // 投影
	};

	// -------------------------------
	// 着色器 1：TextureVertex 的 VS/PS
	// -------------------------------
	struct TextureVertexInput {
		float3 position : POSITION;
		uint color : COLOR;
		float2 uv : TEXCOORD;
		float4 normal : NORMAL;
	};

	struct TextureVSOutput {
		float4 position : SV_POSITION;
		uint color : COLOR;
		float2 uv : TEXCOORD;
		float4 normal : NORMAL;
	};

	// --------------------- //
	// Entry point: vTexture //
	// --------------------- //
	TextureVSOutput vTexture(TextureVertexInput input) {
		TextureVSOutput output;
		output.position = float4(input.position, 1.0f);
		output.color = input.color;
		output.uv = input.uv;
		output.normal = normalize(input.normal);
		return output;
	}

	Texture2D diffuseTexture : register(t0);
	SamplerState textureSampler : register(s0);

	// --------------------- //
	// Entry point: pTexture //
	// --------------------- //
	float4 pTexture(TextureVSOutput input) : SV_TARGET {
		float4 vertexColor = float4(
			float(0x00ff0000 & input.color >> 16) / 255.0,
			float(0x0000ff00 & input.color >> 8 ) / 255.0,
			float(0x000000ff & input.color      ) / 255.0,
			float(0xff000000 & input.color >> 24) / 255.0
		);
		float4 textureColor = diffuseTexture.Sample(textureSampler, input.uv);
		return textureColor * vertexColor;
	}

	// -----------------------------------------------------------------
	// 着色器 2：ColoredVertex 的 VS/PS
	// -----------------------------------------------------------------
	struct ColoredVertexInput {
		float3 position : POSITION;
		uint color : COLOR;
	};

	struct ColoredVSOutput {
		float4 position : SV_POSITION;
		float4 color : COLOR;
	};

	// --------------------- //
	// Entry point: vColored //
	// --------------------- //
	ColoredVSOutput vColored(ColoredVertexInput input) {
		ColoredVSOutput output;
		output.position = mul(g_matrix, float4(input.position, 1.0f));
		output.color = float4(
			float((0x00ff0000 & input.color) >> 16) / 255.0,
			float((0x0000ff00 & input.color) >> 8 ) / 255.0,
			float((0x000000ff & input.color)      ) / 255.0,
			float((0xff000000 & input.color) >> 24) / 255.0
		);
		return output;
	}

	// --------------------- //
	// Entry point: pColored //
	// --------------------- //
	float4 pColored(ColoredVSOutput input) : SV_TARGET {
		return input.color;
	}
)shader";
