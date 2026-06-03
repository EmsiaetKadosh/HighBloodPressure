#define HBP_DX_DEBUG true

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxguid.lib")
#if false
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#endif

#include <map>
#include "direct.hpp"
#include <d3dcompiler.h>
#if HBP_DX_DEBUG
#include <dxgidebug.h>
#endif

#include "src\utils\exception.hpp"
#include "src\render\dx\loader.hpp"
#include "shader.hpp"
#include "src\main.hpp"

#define ifFailed(r, info) if (FAILED(r)) return getLogger().ofNoexcept(info " Error: ", getError(r)).error(), true
#define elseSucceeded(expr) else { expr; }
#define HBP_DX_RESET(com) if (com) (com).Reset()
#define CrashReturn(r, info, ret) if (FAILED(r)) { getLogger().ofNoexcept(info " Error: ", renderer.getError(r)).error(), game.crash(info); return ret; }
#define CrashReturnR(r, info, ret) if (FAILED(r)) { renderer.getLogger().ofNoexcept(info " Error: ", renderer.getError(r)).error(), game.crash(info); return ret; }
#define DiscardReturn(r) (r)

static constexpr D3D12_INPUT_ELEMENT_DESC TEXTURE_LAYOUT[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32_UINT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R16G16B16A16_SINT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};
static constexpr size_t TEXTURE_LAYOUT_SIZE = 4;

static constexpr D3D12_INPUT_ELEMENT_DESC COLORED_LAYOUT[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32_UINT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};
static constexpr size_t COLORED_LAYOUT_SIZE = 2;

void initializeReturnCode(Map<HRESULT, String>& err) {
	if (!err.empty()) return;
	err.emplace(D3D12_ERROR_ADAPTER_NOT_FOUND, L"D3D12_ERROR_ADAPTER_NOT_FOUND: 指定的缓存PSO是在不同的适配器上创建的，不能在当前适配器上重复使用");
	err.emplace(D3D12_ERROR_DRIVER_VERSION_MISMATCH, L"D3D12_ERROR_DRIVER_VERSION_MISMATCH: 指定的缓存PSO是在不同的驱动程序版本上创建的，不能在当前适配器上重复使用");
	err.emplace(DXGI_ERROR_INVALID_CALL, L"DXGI_ERROR_INVALID_CALL: 方法调用无效。例如，方法的参数可能不是有效的指针");
	// errors.emplace(DXGI_ERROR_WAS_STILL_DRAWING, L"DXGI_ERROR_WAS_STILL_DRAWING: 将信息传输到此图面或从此图面传输信息的上一个blit不完整"); // 重复项
	err.emplace(E_FAIL, L"E_FAIL: 尝试创建启用了调试层且未安装该层的设备");
	err.emplace(E_INVALIDARG, L"E_INVALIDARG: 将无效参数传递给返回函数");
	err.emplace(E_OUTOFMEMORY, L"E_OUTOFMEMORY: Direct3D无法分配足够的内存来完成调用");
	err.emplace(E_NOTIMPL, L"E_NOTIMPL: 方法调用不是使用传递的参数组合实现的");
	err.emplace(S_FALSE, L"S_FALSE： 备用成功值，指示成功但非标准完成（精确含义取决于上下文）");
	err.emplace(S_OK, L"S_OK: 未发生错误");
	err.emplace(DXGI_ERROR_ACCESS_DENIED, L"DXGI_ERROR_ACCESS_DENIED: 您尝试使用没有所需访问权限的资源。 此错误通常是在写入具有只读访问权限的共享资源时导致的");
	err.emplace(DXGI_ERROR_ACCESS_LOST, L"DXGI_ERROR_ACCESS_LOST: 桌面重复接口无效。当桌面上显示不同类型的图像时，桌面重复界面通常会失效");
	err.emplace(DXGI_ERROR_ALREADY_EXISTS, L"DXGI_ERROR_ALREADY_EXISTS: 所需的元素已存在。如果不是第一次调用函数，则DXGIDeclareAdapterRemovalSupport会返回此函数");
	err.emplace(DXGI_ERROR_CANNOT_PROTECT_CONTENT, L"DXGI_ERROR_CANNOT_PROTECT_CONTENT: DXGI无法在交换链上提供内容保护。此错误通常是由较旧的驱动程序引起的，或者当你使用与内容保护不兼容的交换链时");
	err.emplace(DXGI_ERROR_DEVICE_HUNG, L"DXGI_ERROR_DEVICE_HUNG: 由于应用程序发送的命令格式不正确，应用程序的设备出现故障。 这是一个设计时问题，应进行调查和修复");
	err.emplace(DXGI_ERROR_DEVICE_REMOVED, L"DXGI_ERROR_DEVICE_REMOVED: 视频卡已实际从系统中删除，或者视频卡的驱动程序升级。 应用程序应销毁并重新创建设备。有关调试问题的帮助，请调用ID3D10Device::GetDeviceRemovedReason");
	err.emplace(DXGI_ERROR_DEVICE_RESET, L"DXGI_ERROR_DEVICE_RESET: 由于命令格式不正确，设备失败。这是一个运行时问题；应用程序应销毁并重新创建设备");
	err.emplace(DXGI_ERROR_DRIVER_INTERNAL_ERROR, L"DXGI_ERROR_DRIVER_INTERNAL_ERROR: 驱动程序遇到问题，并已进入设备删除状态");
	err.emplace(DXGI_ERROR_FRAME_STATISTICS_DISJOINT, L"DXGI_ERROR_FRAME_STATISTICS_DISJOINT: 例如，某个事件（电源周期）中断了当前统计信息的收集");
	err.emplace(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE, L"DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: 应用程序尝试获取输出的独占所有权，但失败，因为应用程序中的一些其他应用程序（或设备）已获取所有权");
	err.emplace(DXGI_ERROR_MORE_DATA, L"DXGI_ERROR_MORE_DATA: 应用程序提供的缓冲区不够大，无法容纳请求的数据");
	err.emplace(DXGI_ERROR_NAME_ALREADY_EXISTS, L"DXGI_ERROR_NAME_ALREADY_EXISTS: 在调用IDXGIResource1::CreateSharedHandle时提供的资源名称已与某些其他资源相关联");
	err.emplace(DXGI_ERROR_NONEXCLUSIVE, L"DXGI_ERROR_NONEXCLUSIVE: 全局计数器资源正在使用中，Direct3D设备当前无法使用计数器资源");
	err.emplace(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE, L"DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: 资源或请求当前不可用，但以后可能会变得可用");
	err.emplace(DXGI_ERROR_NOT_FOUND, L"DXGI_ERROR_NOT_FOUND: 调用IDXGIObject::GetPrivateData 时，不会将传入的 GUID识别为以前传递给IDXGIObject::SetPrivateData或IDXGIObject::SetPrivateDataInterface的GUID。调用IDXGIFactory::EnumAdapters或IDXGIAdapter::EnumOutputs时，枚举的序号已超过范围");
	err.emplace(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED, L"DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED预留");
	err.emplace(DXGI_ERROR_REMOTE_OUTOFMEMORY, L"DXGI_ERROR_REMOTE_OUTOFMEMORY: DXGI_ERROR_REMOTE_OUTOFMEMORY预留");
	err.emplace(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE, L"DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: 交换链内容受限（监视器）的DXGI输出现在已断开连接或更改");
	err.emplace(DXGI_ERROR_SDK_COMPONENT_MISSING, L"DXGI_ERROR_SDK_COMPONENT_MISSING: 操作依赖于缺少或不匹配的SDK组件");
	err.emplace(DXGI_ERROR_SESSION_DISCONNECTED, L"DXGI_ERROR_SESSION_DISCONNECTED: 远程桌面服务会话当前已断开连接");
	err.emplace(DXGI_ERROR_UNSUPPORTED, L"DXGI_ERROR_UNSUPPORTED: 设备或驱动程序不支持请求的功能");
	err.emplace(DXGI_ERROR_WAIT_TIMEOUT, L"DXGI_ERROR_WAIT_TIMEOUT: 在下一个桌面帧可用之前经过的超时间隔");
	err.emplace(DXGI_ERROR_WAS_STILL_DRAWING, L"DXGI_ERROR_WAS_STILL_DRAWING: GPU在调用执行操作时处于繁忙状态，并且未执行或计划操作");
}

void DirectCamera::calculatePitchYaw() noexcept {
	// 最大俯仰
	double constexpr limit = 1.57;
	if (pitch > limit) pitch = limit;
	if (pitch < -limit) pitch = -limit;
	direction.setX(std::cos(pitch) * std::sin(yaw));
	direction.setY(std::sin(pitch));
	direction.setZ(std::cos(pitch) * std::cos(yaw));
}

void DirectCamera::calculate(const double width, const double height) noexcept {
	const Vector3D
		D = direction.getNormalized(),
		R = Vector3D(0, -1, 0).cross(D).normalize(),
		U = R.clone().cross(D).normalize();
	const Matrix4D view = Matrix4D(
		R.getX(), R.getY(), R.getZ(), -(R * position),
		U.getX(), U.getY(), U.getZ(), -(U * position),
		D.getX(), D.getY(), D.getZ(), -(D * position),
		0, 0, 0, 1
	);
	Matrix4D proj;
	const double vt = 1 / std::tan(fieldOfView * 0.5); // 投影系数
	if (projection == Perspective) {                   // 景深投影
		const double aspectReversed = height / width;    // 反宽高比
		proj = Matrix4D(
			aspectReversed * vt, 0, 0, 0,
			0, vt, 0, 0,
			0, 0, farthestDistance / (farthestDistance - nearestDistance), -nearestDistance * farthestDistance / (farthestDistance - nearestDistance),
			0, 0, 1, 0
		);
	}
	else // 平行投影
		proj = Matrix4D(
			2.0 / width, 0, 0, 0,
			0, 2.0 / height, 0, 0,
			0, 0, -2.0 / (farthestDistance - nearestDistance), -(farthestDistance + nearestDistance) / (farthestDistance - nearestDistance),
			0, 0, 0, 1
		);
	DirectMatrixFromMatrix(proj.multiply(view).transpose(), conMatrix); // 获得转换矩阵
}

bool DirectResource::resize(unsigned long long& amount, const size_t unitSize) noexcept {
	if (currentSize > amount * unitSize) return false;
	amount >>= 2, amount *= 5;
	currentSize = amount * unitSize;
	return resize(currentSize);
}

bool DirectResource::resize(const size_t unitSize) noexcept {
	if (unitSize < currentSize) return false;
	if (status == DirectResourceStatus::Map) {
		renderer.getLogger().ofNoexcept(L"Resource status not matched when resizing: ", FunctionSignature()).error();
		game.getRiskManager().report();
	}
	buffer.Reset();
	const D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const D3D12_RESOURCE_DESC indexDesc = CD3DX12_RESOURCE_DESC::Buffer(currentSize); // UINT1
	HRESULT hr = 0;
	hr = renderer.device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &indexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));
	if (FAILED(hr)) return renderer.getLogger().ofNoexcept(L"Failed to reassign for coloredIndexBuffer. Error: ", renderer.getError(hr)).error(), true;
	return false;
}

char* DirectResource::map() noexcept {
	if (status == DirectResourceStatus::Map) {
		renderer.getLogger().error(L"Resource status not matched when mapping");
		game.getRiskManager().report();
	}
	char* ret;
	HRESULT result = 0;
	result = buffer->Map(0, nullptr, reinterpret_cast<void**>(&ret));
	CrashReturnR(result, L"Failed to map resource.", nullptr);
	status = DirectResourceStatus::Map;
	return ret;
}

void DirectResource::unmap() noexcept {
	if (status == DirectResourceStatus::Unmap) {
		renderer.getLogger().error(L"Resource status not matched when unmapping");
		game.getRiskManager().report();
	}
	buffer->Unmap(0, nullptr);
	status = DirectResourceStatus::Unmap;
}

void DirectResource::finalize() noexcept {
	if (status == DirectResourceStatus::Map) unmap();
	HBP_DX_RESET(buffer);
}

ID3D12Resource* DirectResource::getBuffer() const noexcept { return buffer.Get(); }

bool DirectTextureResource::resize(unsigned int singleWidth, const unsigned int singleHeight, const unsigned int length) noexcept {
	if (singleWidth == 0 || singleHeight == 0) return true;
	if (constexpr unsigned int aln = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT / sizeof(unsigned int); singleWidth % aln) singleWidth = (singleWidth / aln + 1) * aln; // GPU Alignment
	if (width == singleWidth && height == singleHeight && size == length) return true;
	finalize();
	HRESULT hr = 0;
	width = singleWidth;
	height = singleHeight;
	size = length;
	// 1. 创建纹理描述
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.DepthOrArraySize = size; // 数组个数
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ARGB对应
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	// 2. 创建默认堆纹理（GPU显存）。
	// 初始时设为PSR，每次上传前转换为CD，上传后转回PSR；空闲时间、创建时、析构时均保持PSR
	const CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	hr = renderer.device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&defaultBuffer));
	if (FAILED(hr)) {
		renderer.getLogger().ofNoexcept(L"Failed to create default texture heap: ", renderer.getError(hr)).error();
		game.getRiskManager().report();
		return true;
	}
	// 3. 计算上传所需的大小和行对齐
	renderer.device->GetCopyableFootprints(&textureDesc, 0, 1, 0, &footprint, &numRows, &rowPitch, &uploadBufferSize);
	// 4. 创建上传堆（CPU可写）
	const CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	const CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	hr = renderer.device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
	if (FAILED(hr)) {
		renderer.getLogger().ofNoexcept(L"Failed to create upload texture heap: ", renderer.getError(hr)).error();
		game.getRiskManager().report();
		return true;
	}
	mapAddress = map();
	return false;
}

char* DirectTextureResource::map() noexcept {
	if (status == DirectResourceStatus::Map) {
		renderer.getLogger().error(L"TextureResource status not matched when mapping");
		game.getRiskManager().report();
	}
	char* ret;
	HRESULT result = 0;
	result = uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&ret));
	CrashReturnR(result, L"Failed to map resource.", nullptr);
	status = DirectResourceStatus::Map;
	return ret;
}

void DirectTextureResource::unmap() noexcept {
	if (status == DirectResourceStatus::Unmap) {
		renderer.getLogger().error(L"TextureResource status not matched when unmapping");
		game.getRiskManager().report();
	}
	uploadBuffer->Unmap(0, nullptr);
	status = DirectResourceStatus::Unmap;
}

void DirectTextureResource::submit(const ComPtr<ID3D12GraphicsCommandList>& commandList) const noexcept {
	if (!defaultBuffer || !uploadBuffer) return;
	// 1. 转换状态：PSR -> CD
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(1, &barrier);
	const UINT64 perSubresourceSize = numRows * rowPitch; // 单个子资源占用的字节数
	// 2. 对于每个要复制的子资源
	// TODO(EmsiaetKadosh): 改为仅上传被修改的资源
	for (UINT i = 0; i < size; ++i) {
		const UINT subresource = i;
		D3D12_TEXTURE_COPY_LOCATION dst = {}; // 目标位置：子资源索引
		dst.pResource = defaultBuffer.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = subresource;
		D3D12_TEXTURE_COPY_LOCATION src = {}; // 源位置：使用预先计算的 footprint
		src.pResource = uploadBuffer.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = footprint; // 注意：footprint 是针对单个子资源的
		// 对于纹理数组，每个子资源的源数据在 uploadBuffer 中的偏移需要累加
		// 假设 footprint.Offset 是第一个子资源的起始偏移，每个子资源占用固定大小
		src.PlacedFootprint.Offset += i * perSubresourceSize;
		// 执行复制（复制整个纹理）
		commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	}
	// 3. 恢复状态：CD -> PSR
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);
}

void DirectTextureResource::finalize() noexcept {
	if (status == DirectResourceStatus::Map && uploadBuffer) {
		uploadBuffer->Unmap(0, nullptr);
		mapAddress = nullptr;
	}
	HBP_DX_RESET(uploadBuffer);
	HBP_DX_RESET(defaultBuffer);
	status = DirectResourceStatus::Null;
}

bool DirectTextureResource::fillRect(const unsigned int index, const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h, const unsigned int* data) const noexcept {
	if (status != DirectResourceStatus::Map || index >= size || !data) return false;
	if (x + w > width || y + h > height) return false; // 边界检查

	// 计算目标纹理在upload buffer中的起始偏移
	const size_t textureBaseOffset = index * (numRows * rowPitch) + footprint.Offset;

	// 逐行复制部分区域
	for (size_t row = 0; row < h; ++row) {
		const size_t targetRow = y + row;
		const size_t targetOffset = textureBaseOffset + targetRow * rowPitch + x * sizeof(uint32_t);

		memcpy(
			mapAddress + targetOffset,
			reinterpret_cast<const char*>(data) + row * w * sizeof(uint32_t),
			w * sizeof(uint32_t)
		);
	}

	return true;
}

void DirectTextureDispatcher::updateAtlasFillRate(Atlas& atlas) noexcept { atlas.fillRate = static_cast<float>(atlas.usedCells) / static_cast<float>(atlas.grid.size()); }

void DirectTextureDispatcher::createAtlas(Category category) noexcept { atlases.emplace_back(category, manager.getGridSize()); }

DirectTextureDispatcher::Category DirectTextureDispatcher::categoryOf(const DirectTextureCarrier* const carrier) const noexcept { return carrier->width < manager.getUnitSize() && carrier->height < manager.getUnitSize() ? Unit : Special; }

std::pair<unsigned int, unsigned> DirectTextureDispatcher::indexToXY(const unsigned int index) const noexcept {
	unsigned int d = index / manager.getUnitSize();
	unsigned int m = index % manager.getUnitSize();
	// 注意此处y是商（分片），x是余数（连续）
	return std::make_pair(m, d);
}

bool DirectTextureDispatcher::isLineEmpty(Atlas& atlas, const unsigned int x, const unsigned int y, const unsigned int width) const noexcept {
	const unsigned int gridSize = manager.getGridSize();
	unsigned int end = x + width;
	if (end > gridSize) return false; // x方向越界。下面的边界符号是小于，所以此处允许等于
	end += y * gridSize;
	for (unsigned int i = x + y * gridSize; i < end; ++i) if (atlas.grid[i]) return false;
	return true;
}

bool DirectTextureDispatcher::isColumnEmpty(Atlas& atlas, const unsigned int x, const unsigned int y, const unsigned int height) const noexcept {
	const unsigned int gridSize = manager.getGridSize();
	unsigned int end = y + height;
	if (end > gridSize) return false;
	end = end * gridSize + x;
	for (unsigned int i = x + y * gridSize; i < end; i += gridSize) if (atlas.grid[i]) return false;
	return true;
}

DirectTextureIndex DirectTextureDispatcher::newPrepare(const DirectTextureCarrier* const carrier) const noexcept {
	const Category category = categoryOf(carrier);
	DirectTextureIndex ret;
	ret.xSampleSize = static_cast<float>(carrier->width) / static_cast<float>(manager.getSpecialSize());
	ret.ySampleSize = static_cast<float>(carrier->height) / static_cast<float>(manager.getSpecialSize());
	if (category == Unit) { // Unit
		ret.buffer = buffer;
		ret.xGridSize = 1;
		ret.yGridSize = 1;
	}
	else { // Special
		ret.buffer = buffer;
		// 两个ceil
		ret.xGridSize = 1 + (carrier->width - 1) / manager.getUnitSize();
		ret.yGridSize = 1 + (carrier->height - 1) / manager.getUnitSize();
	}
	return ret;
}

void DirectTextureDispatcher::newAdapt(const DirectTextureCarrier* const carrier, DirectTextureIndex& index) noexcept {
	const Category category = categoryOf(carrier);
	if (category == Unit) {
		for (unsigned int i = 0; i < atlases.size(); ++i) {
			const Atlas& atlas = atlases[i];
			if (atlas.category != Unit) continue;
			if (atlas.fillRate >= manager.getFillThreshold()) continue;
			if (fitForUnit(i, index, carrier)) return;
		}
		for (unsigned int i = 0; i < atlases.size(); ++i) {
			const Atlas& atlas = atlases[i];
			if (atlas.category != Special) continue;
			if (atlas.fillRate >= manager.getFillThreshold()) continue;
			if (fitForUnit(i, index, carrier)) return;
		}
	}
	else
		for (unsigned int i = 0; i < atlases.size(); ++i) {
			const Atlas& atlas = atlases[i];
			if (atlas.category != Special) continue;
			if (atlas.fillRate >= manager.getFillThreshold()) continue;
			if (fitForSpecial(i, index, carrier)) return;
		}
	// 都不接受，那么创建一个新的
	createAtlas(category);
	const auto func = category == Unit ? &DirectTextureDispatcher::fitForUnit : &DirectTextureDispatcher::fitForSpecial;
	if (const bool flag = (this->*func)(atlases.size() - 1, index, carrier); !flag) {
		renderer.getLogger().ofNoexcept(L"Failed to fit into new atlas: ", FunctionSignature()).error();
		game.getRiskManager().report();
	}
}

bool DirectTextureDispatcher::fitForUnit(const unsigned int indexAtlas, DirectTextureIndex& index, [[maybe_unused]] const DirectTextureCarrier* carrier) noexcept {
	if (index.xGrid != 1 || index.yGrid != 1) return false;
	Atlas& atlas = atlases[indexAtlas];
	const unsigned int start = atlas.category == Unit || atlas.grid.back() ? 0 : atlas.usedCells;
	// 如果末尾已经填充（认为已经填充过一轮），或者借用Spec拼合图集，就从0开始
	// 否则是未一轮填充的单元图集，从usedCells开始，理论上最多到end就应当结束
	for (unsigned int i = start; i < atlas.grid.size(); ++i) {
		if (atlas.grid[i]) continue;
		atlas.grid[i] = true;
		const auto& [x, y] = indexToXY(i);
		index.xGrid = x;
		index.yGrid = y;
		index.xSample = static_cast<float>(index.xGrid * manager.getGridSize()) / static_cast<float>(manager.getSpecialSize());
		index.ySample = static_cast<float>(index.yGrid * manager.getGridSize()) / static_cast<float>(manager.getSpecialSize());
		index.index = indexAtlas;
		++atlas.usedCells;
		updateAtlasFillRate(atlas);
		return true;
	}
	renderer.getLogger().ofNoexcept(L"Failed to fit into atlas (unit): ", carrier->getFile()).error();
	game.getRiskManager().report();
	return false;
}

bool DirectTextureDispatcher::fitForSpecial(const unsigned int indexAtlas, DirectTextureIndex& index, const DirectTextureCarrier* carrier) noexcept {
	// 计算所需网格数
	const unsigned int width = index.xGridSize;
	const unsigned int height = index.yGridSize;
	Atlas& atlas = atlases[indexAtlas];
	// 检查纹理是否太大（不应该发生，但安全检查）
	if (carrier->width > manager.getSpecialSize() || carrier->height > manager.getSpecialSize()) {
		renderer.getLogger().ofNoexcept(L"Failed to fit into atlas (special); texture too large: ").error();
		game.getRiskManager().report();
		return false;
	}
	// 检查填充率阈值
	if (atlas.fillRate != 0) { // 如果非空
		// ReSharper disable once CppTooWideScopeInitStatement
		const float newFillRate = static_cast<float>(atlas.usedCells + width * height) / atlas.grid.size();
		if (newFillRate > manager.getFillThreshold()) return false; // 填充过于困难
	}

	const unsigned int res = carrier->width % manager.getUnitSize();
	const bool isEdgeAvailable = res <= manager.getEdgeSize();

	// 查找连续空闲网格
	const unsigned int xTarget = manager.getGridSize() - width; // x和y上的查找最大值
	const unsigned int yTarget = manager.getGridSize() - height;
	const unsigned int ew = width - 1, eh = height - 1;
	unsigned int xResult, yResult;
	if (isEdgeAvailable && width > 1 && height > 1) { // 当边界可用时，优先查找边界用法
		// 此处做一个安全保障的unreachable筛选，如果width和height任一是1，取消边界查找，防止内存失控
		// 边界查找时，xTarget和yTarget的值+1
		const unsigned int ext = xTarget + 1, eyt = yTarget + 1;
		unsigned int stacking = 0; // 行/列累计的空置区域计数
		unsigned int startPoint = 0;
		while (startPoint <= eyt) {
			while (stacking < height) // 检查位于起始点(ext, startPoint)是否全空
				if (isLineEmpty(atlas, ext, startPoint + stacking, ew)) ++stacking;
				else {                                    // 非空
					startPoint = startPoint + stacking + 1; // 下一行开始
					stacking = 0;
					break;
				}
			if (stacking >= height) {
				xResult = ext;
				yResult = startPoint;
				index.xGridSize = ew;
				index.yGridSize = startPoint == eyt ? eh : height;
				index.index = indexAtlas;
				goto ok;
			}
		}
		startPoint = 0;
		stacking = 0;
		while (startPoint < ext) { // 此处无需再检查ext
			while (stacking < width)
				if (isColumnEmpty(atlas, startPoint + stacking, eyt, eh)) ++stacking;
				else {
					startPoint = startPoint + stacking + 1;
					stacking = 0;
					break;
				}
			if (stacking >= width) {
				xResult = startPoint;
				yResult = eyt;
				index.xGridSize = width;
				index.yGridSize = eh;
				index.index = indexAtlas;
				goto ok;
			}
		}
	}
	// 常规检查空置区域
	// 通配符覆盖卷积法
	{
		Vector<bool> area;
		area.resize(width + eh * manager.getGridSize(), false); // 一维化做卷积
		for (unsigned int i = 0; i < height; ++i) std::fill_n(area.begin() + i * manager.getGridSize(), width, true);
		// TODO(EmsiaetKadosh): 进一步优化
		for (unsigned int i = 0; i < atlas.grid.size() - area.size(); ++i) {
			bool flag = true;
			for (unsigned int j = 0; j < area.size(); ++j) {
				if (!area[j]) continue; // 通配符
				if (area[j + i]) {      // 需要空置时非空
					flag = false;
					break;
				}
			}
			if (flag) { // 匹配成功
				auto [xr, yr] = indexToXY(i);
				xResult = xr, yResult = yr;
				index.index = indexAtlas;
				goto ok;
			}
		}
	}
	return false; // 查找失败
ok:
	index.xGrid = xResult;
	index.yGrid = yResult;
	index.xSample = static_cast<float>(index.xGrid * manager.getGridSize()) / static_cast<float>(manager.getSpecialSize());
	index.ySample = static_cast<float>(index.yGrid * manager.getGridSize()) / static_cast<float>(manager.getSpecialSize());

	setRange(index, true);
	atlas.usedCells = index.xGridSize * index.yGridSize; // 边界情况会修改此二值，直接用此二值计算
	updateAtlasFillRate(atlas);
	return true;
}

void DirectTextureDispatcher::update() noexcept {}

void DirectTextureDispatcher::apply(DirectTextureResource& resource) noexcept {
	for (auto& [t, i]: indices) {
		t->prepare();
		const unsigned int* b = reinterpret_cast<const unsigned int*>(t->extraData->getBuffer());
		if (!b) continue;
		if (resource.fillRect(i.index, i.xGrid, i.yGrid, i.xGridSize, i.yGridSize, b)) continue;
		renderer.getLogger().ofNoexcept(L"Failed to copy Texture to upload-buffer: File[", t->file, "] @ ", FunctionSignature()).error();
	}
}

const DirectTextureIndex& DirectTextureDispatcher::getIndex(DirectTextureCarrier* const carrier) noexcept {
	const auto iter = indices.find(carrier);
	if (iter == indices.end()) return DirectTextureIndex::ofNull();
	return iter->second;
}

// TODO(EmsiaetKadosh): prepare可能需要检查一些更多状态
void DirectTextureResourceManager::prepareConstant(DirectTextureResource& buffer) noexcept {
	if (renderer.checkThread()) {
		renderer.getLogger().ofNoexcept(L"Texture preparation must be at RenderThread: ", FunctionSignature()).error();
		return;
	}
	renderer.awaitAllFrames(); // 确保没有执行渲染
	dispatcherConstant.apply(buffer);
}

void DirectTextureResourceManager::preparePreload(DirectTextureResource& buffer) noexcept {
	if (renderer.checkThread()) {
		renderer.getLogger().ofNoexcept(L"Texture preparation must be at RenderThread: ", FunctionSignature()).error();
		return;
	}
	renderer.awaitAllFrames(); // 确保没有执行渲染
	dispatcherPreload.apply(buffer);
}

bool DirectTextureResourceManager::prepareAppend(DirectTextureResource& buffer) noexcept {
	if (renderer.checkThread()) {
		renderer.getLogger().ofNoexcept(L"Texture preparation must be at RenderThread: ", FunctionSignature()).error();
		return true;
	}
	if (not buffer.isOutdated(getCurrentScene().version)) return false;
	dispatcherAppend.apply(buffer);
	return false;
}

bool DirectTextureResourceManager::prepareTemporary(DirectTextureResource& buffer) noexcept {
	if (renderer.checkThread()) {
		renderer.getLogger().ofNoexcept(L"Texture preparation must be at RenderThread: ", FunctionSignature()).error();
		return true;
	}
	dispatcherTemporary.apply(buffer);
	return false;
}

bool DirectFrame::reassignColoredBuffer(const size_t size) noexcept { return coloredVertex.resize(maxColoredVertexCount = size, sizeof(ColoredVertex)); }
bool DirectFrame::reassignTextureBuffer(const size_t size) noexcept { return textureVertex.resize(maxTextureVertexCount = size, sizeof(TextureVertex)); }
bool DirectFrame::reassignColoredIndexBuffer(const size_t size) noexcept { return coloredIndex.resize(maxColoredIndexCount = size, sizeof(unsigned int)); }

bool DirectFrame::reassignDepthStencil() noexcept {
	HRESULT result = 0;

	const D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		renderer.getWidth(), renderer.getHeight(),
		1, 0, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil = {1.0f, 0};
	const CD3DX12_HEAP_PROPERTIES properties {D3D12_HEAP_TYPE_DEFAULT};
	result = renderer.device->CreateCommittedResource(
		&properties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilView)
	);
	if (FAILED(result)) return renderer.getLogger().ofNoexcept(L"Failed to create dsv buffer. Error: ", result).error(), true;
	// 创建深度视图
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	renderer.device->CreateDepthStencilView(depthStencilView.Get(), &dsvDesc, renderer.dsvHeap->GetCPUDescriptorHandleForHeapStart());
	return false;
}

bool DirectFrame::initialize(unsigned int index, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& dsvHandle) {
	HRESULT hr = 0;
	hr = renderer.swapChain->GetBuffer(index, IID_PPV_ARGS(&renderTargetView));
	if (FAILED(hr)) return renderer.getLogger().ofNoexcept(L"Failed to get rtv buffer: n = ", index, L". Error: ", renderer.getError(hr)).error(), true;
	if (reassignDepthStencil()) return true;

	renderer.device->CreateRenderTargetView(renderTargetView.Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, renderer.rtvDescriptorSize);
	renderer.device->CreateDepthStencilView(depthStencilView.Get(), nullptr, dsvHandle);
	dsvHandle.Offset(1, renderer.dsvDescriptorSize);

	hr = renderer.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr)) return renderer.getLogger().ofNoexcept(L"Failed to create command allocator: n = ", index, L". Error: ", renderer.getError(hr)).error(), true;
	hr = renderer.device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList));
	if (FAILED(hr)) return renderer.getLogger().ofNoexcept(L"Failed to create command list: n = ", index, L". Error: ", renderer.getError(hr)).error(), true;

	if (reassignColoredBuffer(1024)) return true;
	if (reassignTextureBuffer(1024)) return true;
	if (reassignColoredIndexBuffer(1024)) return true;
	if (constants.resize(sizeof(CameraMatrix))) return true;
	return false;
}

void DirectFrame::awaitFrame() noexcept {
	if (state >= Completed) return;
	while (state < Process) _mm_pause();
	renderer.awaitSignal(thisSignal);
	// renderer.getLogger().trace(L"Signal complete (AwaitFrame): " + std::to_wstring(thisSignal));
	state = Completed;
}

void DirectFrame::finalize() noexcept {
	awaitFrame();
	HBP_DX_RESET(commandList);
	HBP_DX_RESET(commandAllocator);
	constants.finalize();
	coloredVertex.finalize();
	textureVertex.finalize();
	coloredIndex.finalize();
	textureAppend.finalize();
	textureTemporary.finalize();
	HBP_DX_RESET(renderTargetView);
	HBP_DX_RESET(depthStencilView);
}

void DirectFrame::debugCustom(unsigned int index) noexcept {
	renderer.getLogger().ofNoexcept(L"DirectFrame @ index ", index).debug();
	renderer.getLogger().ofNoexcept(L"    commandAllocator @ ", ptrtow(commandAllocator.Get())).debug();
	renderer.getLogger().ofNoexcept(L"    commandList @ ", ptrtow(commandList.Get())).debug();
	renderer.getLogger().ofNoexcept(L"    constants @ ", ptrtow(constants.getBuffer())).debug();
	renderer.getLogger().ofNoexcept(L"    coloredVertex @ ", ptrtow(coloredVertex.getBuffer())).debug();
	renderer.getLogger().ofNoexcept(L"    textureVertex @ ", ptrtow(textureVertex.getBuffer())).debug();
	renderer.getLogger().ofNoexcept(L"    coloredIndex @ ", ptrtow(coloredIndex.getBuffer())).debug();
	renderer.getLogger().ofNoexcept(L"    renderTargetView @ ", ptrtow(renderTargetView.Get())).debug();
	renderer.getLogger().ofNoexcept(L"    thisSignal = ", thisSignal).debug();
	renderer.getLogger().ofNoexcept(L"    maxColoredIndexCount = ", maxColoredIndexCount).debug();
	renderer.getLogger().ofNoexcept(L"    maxColoredVertexCount = ", maxColoredVertexCount).debug();
	renderer.getLogger().ofNoexcept(L"    maxTextureVertexCount = ", maxTextureVertexCount).debug();
	renderer.getLogger().ofNoexcept(L"    actualColoredIndexCount = ", actualColoredIndexCount).debug();
	renderer.getLogger().ofNoexcept(L"    actualColoredVertexCount = ", actualColoredVertexCount).debug();
	renderer.getLogger().ofNoexcept(L"    actualTextureVertexCount = ", actualTextureVertexCount).debug();
}

void DirectFrame::begin() noexcept(false) {
	awaitFrame();
	state = Await;
	HRESULT result = 0;
	// 1. 重置命令列表和分配器
	result = commandAllocator->Reset();
	CrashReturnR(result, L"Failed to reset command allocator.",);
	result = commandList->Reset(commandAllocator.Get(), renderer.coloredPipelineState.Get());
	CrashReturnR(result, L"Failed to reset command list.",);
	{
		const D3D12_RESOURCE_BARRIER renderBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargetView.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &renderBarrier); // 启动渲染前，需要设置成RENDER_TARGET状态
	}
	// 2. 设置视口和裁剪区域
	commandList->RSSetViewports(1, &renderer.viewport);
	commandList->RSSetScissorRects(1, &renderer.scissorRect);
	// 3. 设置渲染目标
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(renderer.rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(renderer.currentFrame), renderer.rtvDescriptorSize);
	const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(renderer.dsvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(renderer.currentFrame), renderer.dsvDescriptorSize);
	commandList->OMSetRenderTargets(1, &rtvHandle, 0, &dsvHandle);
	// 4. 清除渲染目标
	constexpr float clearColor[] = {0, 0, 0, 1.0f};
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// 5.0. 设置图形管线状态
	commandList->SetGraphicsRootSignature(renderer.rootSignature.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	state = Accept;
}

bool DirectFrame::submitColoredVertices() noexcept {
	if (!actualColoredVertexCount) return false;
	if (!actualColoredIndexCount) return false;
	if (actualColoredVertexCount >= maxColoredVertexCount) if (reassignColoredBuffer(actualColoredVertexCount)) return true;
	if (actualColoredIndexCount >= maxColoredIndexCount) if (reassignColoredIndexBuffer(actualColoredIndexCount)) return true;
	char* vertex = coloredVertex.map();
	if (!vertex) return true;
	char* index = coloredIndex.map();
	if (!index) return true;
	for (const auto& [vertices, indices]: coloredBuffer) {
		memcpy(vertex, vertices.data(), vertices.size() * sizeof(ColoredVertex));
		vertex += vertices.size() * sizeof(ColoredVertex);
		memcpy(index, indices.data(), indices.size() * sizeof(unsigned int));
		index += indices.size() * sizeof(unsigned int);
	}
	coloredVertex.unmap();
	coloredIndex.unmap();
	commandList->SetPipelineState(renderer.coloredPipelineState.Get());
	const D3D12_VERTEX_BUFFER_VIEW coloredView = {
		.BufferLocation = coloredVertex.getBuffer()->GetGPUVirtualAddress(),
		.SizeInBytes = static_cast<unsigned int>(actualColoredVertexCount * sizeof(ColoredVertex)),
		.StrideInBytes = sizeof(ColoredVertex)
	};
	commandList->IASetVertexBuffers(0, 1, &coloredView);
	const D3D12_INDEX_BUFFER_VIEW coloredView2 = {
		.BufferLocation = coloredIndex.getBuffer()->GetGPUVirtualAddress(),
		.SizeInBytes = static_cast<unsigned int>(actualColoredIndexCount * sizeof(unsigned int)),
		.Format = DXGI_FORMAT_R32_UINT
	};
	commandList->IASetIndexBuffer(&coloredView2);
	commandList->DrawIndexedInstanced(actualColoredIndexCount, 1, 0, 0, 0);
	actualColoredVertexCount = 0;
	actualColoredIndexCount = 0;
	coloredBuffer.clear();
	return false;
}

bool DirectFrame::submitTextureVertices() noexcept {
	if (!actualTextureVertexCount) return false;
	if (actualTextureVertexCount >= maxTextureVertexCount) if (reassignTextureBuffer(actualTextureVertexCount)) return true;
	// TODO(EmsiaetKadosh): 此处需要再次处理顶点：根据具体安排以后的位置确定具体的采样区间
	char* vertex = textureVertex.map();
	if (!vertex) return true;
	for (const auto& [vertices, indices]: textureBuffer) {
		memcpy(vertex, vertices.data(), vertices.size() * sizeof(TextureVertex));
		vertex += vertices.size() * sizeof(TextureVertex);
	}
	textureVertex.unmap();
	commandList->SetPipelineState(renderer.texturePipelineState.Get());
	const D3D12_VERTEX_BUFFER_VIEW vertexView = {
		.BufferLocation = textureVertex.getBuffer()->GetGPUVirtualAddress(),
		.SizeInBytes = static_cast<unsigned int>(actualTextureVertexCount * sizeof(ColoredVertex)),
		.StrideInBytes = sizeof(ColoredVertex)
	};
	commandList->IASetVertexBuffers(0, 1, &vertexView);
	// const D3D12_INDEX_BUFFER_VIEW coloredView2 = {
	// 	.BufferLocation = coloredIndex.getBuffer()->GetGPUVirtualAddress(),
	// 	.SizeInBytes = static_cast<unsigned int>(actualTextureVertexCount * sizeof(unsigned int)),
	// 	.Format = DXGI_FORMAT_R32_UINT
	// };
	// commandList->IASetIndexBuffer(&coloredView2);
	commandList->DrawInstanced(actualTextureVertexCount, 1, 0, 0);
	actualTextureVertexCount = 0;
	textureBuffer.clear();
	return false;
}

bool DirectFrame::submitTextures() noexcept {
	if (renderer.resourceManager.prepareAppend(textureAppend)) textureAppend.submit(commandList);
	if (renderer.resourceManager.prepareTemporary(textureTemporary)) textureTemporary.submit(commandList);
	return false;
}

void DirectFrame::end() {
	assertStatus(Accept, true);
	// 5. 计算相机；上传常量堆
	{
		char* buffer = constants.map();
		if (!buffer) return;
		if (renderer.camera.consumeChanges()) {
			renderer.camera.calculate(renderer.viewport.Width, renderer.viewport.Height);
			renderer.camera.copy(buffer);
			commandList->SetGraphicsRootConstantBufferView(0, constants.getBuffer()->GetGPUVirtualAddress()); // 传给GPU
		}
		buffer += sizeof(CameraMatrix);
		constants.unmap();
	}
	// 6. 提交顶点
	submitColoredVertices();
	submitTextures();
	submitTextureVertices();
	state = Upload;
	HRESULT result = 0;
	// 7. 提交命令列表
	{
		const D3D12_RESOURCE_BARRIER renderBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargetView.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &renderBarrier); // 切换为PRESENT再Present
	}
	result = commandList->Close();
	CrashReturnR(result, L"Failed to close command list.",);
	ID3D12CommandList* cmdLists[] = {commandList.Get()};
	renderer.commandQueue->ExecuteCommandLists(1, cmdLists);
	// 8. 呈现交换链
	result = renderer.swapChain->Present(1, 0);
	CrashReturnR(result, L"Failed to present swap chain.",);
	thisSignal = renderer.requestSignal();
	state = Process;
}

void DirectFrame::assertStatus(const State expected, const bool strict) const noexcept(false) {
	if (state == expected) return;
	if (strict || state < expected) throw ThreadInterferenceException(L"Current state is ... but expected ...");
}

void DirectFrame::drawColor(ColoredSet&& set) noexcept(false) {
	assertStatus(Accept, true);
	coloredBuffer.push_back(std::move(set));
	auto& [vertices, indices] = coloredBuffer.back();
	for (unsigned int& i: indices) i += actualColoredVertexCount;
	actualColoredVertexCount += vertices.size();
	actualColoredIndexCount += indices.size();
}

void DirectFrame::drawColor(const ColoredSet& set) noexcept(false) {
	assertStatus(Accept, true);
	coloredBuffer.push_back(set);
	auto& [vertices, indices] = coloredBuffer.back();
	for (unsigned int& i: indices) i += actualColoredVertexCount;
	actualColoredVertexCount += vertices.size();
	actualColoredIndexCount += indices.size();
}

bool DirectRenderer::initializeFactory() noexcept {
	getLogger().trace(L"Initializing factory");
	HRESULT result;
	int factoryFlag = 0;
	if constexpr (HBP_DX_DEBUG) {
		ComPtr<ID3D12Debug1> debug;
		result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		ifFailed(result, L"Failed to get debug layer.");
		elseSucceeded(debug->EnableDebugLayer(); debug->SetEnableSynchronizedCommandQueueValidation(true); factoryFlag = DXGI_CREATE_FACTORY_DEBUG; getLogger().info(L"DXGI debug layer enabled")); // NOLINT(*-else-after-return)
	}

	result = CreateDXGIFactory2(factoryFlag, IID_PPV_ARGS(&factory));
	ifFailed(result, L"Failed to create DXGI factory.");
	return false;
}

bool DirectRenderer::initializeDevice(const bool requestHighPerformance) noexcept {
	if (!factory) return true;
	getLogger().trace(L"Initializing device");
	const DXGI_GPU_PREFERENCE flag = requestHighPerformance ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;
	HRESULT result = 0;
	ComPtr<IDXGIAdapter1> adapter;
	for (unsigned int index = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(index, flag, IID_PPV_ARGS(&adapter))); ++index) { // 按性能枚举适配器
		DXGI_ADAPTER_DESC1 desc;
		result = adapter->GetDesc1(&desc);
		if (FAILED(result)) continue;
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr))) break;
	}
	if (!adapter) {
		getLogger().error(L"No suitable adapter found.");
		return true;
	}
	result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
	ifFailed(result, L"Failed to create device.");
#if true // 检查光线追踪支持等新特性。
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 features = {};
	if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features, sizeof(features)))) if (features.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) getLogger().warn(L"Raytracing not supported!");
#endif
	return false;
}

bool DirectRenderer::initializeCommandQueue() noexcept {
	getLogger().trace(L"Initializing command queue");
	HRESULT result = 0;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
	ifFailed(result, L"Failed to create command queue.");
	return false;
}

bool DirectRenderer::initializeSwapChain(const HWND hwnd) noexcept { // NOLINT(*-make-member-function-const)
	getLogger().trace(L"Initializing swap chain");
	HRESULT result = 0;
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.BufferCount = swapFrameCount;
	desc.Width = getWidth();
	desc.Height = getHeight();
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> chain;
	result = factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &desc, nullptr, nullptr, &chain);
	ifFailed(result, L"Failed to create swap chain.");
	result = chain.As(&swapChain);
	ifFailed(result, L"Failed to cast swap chain");
	return false;
}

bool DirectRenderer::initializeHeap() noexcept { // NOLINT(*-make-member-function-const)
	getLogger().trace(L"Initializing heap");
	HRESULT result = 0;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = swapFrameCount;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap));
	ifFailed(result, L"Failed to create rtv heap.");
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	desc = {};
	desc.NumDescriptors = swapFrameCount;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsvHeap));
	ifFailed(result, L"Failed to create dsv heap.");
	return false;
}

bool DirectRenderer::initializeFence() noexcept {
	getLogger().trace(L"Initializing fence");
	HRESULT hr = 0;
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	ifFailed(hr, L"Failed to create fence.");
	fenceValue = 1;
	fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr) {
		ifFailed(HRESULT_FROM_WIN32(GetLastError()), L"Failed to create fence event.");
		return true; // NOLINT(*-simplify-boolean-expr)
	}
	return false;
}

bool DirectRenderer::initializeShader() noexcept {
	getLogger().trace(L"Initializing shader");
	ComPtr<ID3DBlob> err;
	HRESULT hr = 0;
	constexpr const char* colored = HBP_SHADER_COLORED;
	constexpr const char* texture = HBP_SHADER_TEXTURE;
	const size_t sizeColored = std::strlen(colored);
	const size_t sizeTexture = std::strlen(texture);
	//                                      .......  ....... 分别是：源文件名（调试用）；宏定义
	hr = D3DCompile(colored, sizeColored, nullptr, nullptr, nullptr, "vColored", "vs_5_1", 0, 0, &coloredVertexShader, &err);
	ifFailed(hr, L"Failed to compile colored VS.");
	hr = D3DCompile(colored, sizeColored, nullptr, nullptr, nullptr, "pColored", "ps_5_1", 0, 0, &coloredPixelShader, &err);
	ifFailed(hr, L"Failed to compile colored PS.");
	hr = D3DCompile(texture, sizeTexture, nullptr, nullptr, nullptr, "vTexture", "vs_5_1", 0, 0, &textureVertexShader, &err);
	ifFailed(hr, L"Failed to compile texture VS.");
	hr = D3DCompile(texture, sizeTexture, nullptr, nullptr, nullptr, "pTexture", "ps_5_1", 0, 0, &texturePixelShader, &err);
	ifFailed(hr, L"Failed to compile texture PS.");
	return false;
}

inline bool DirectRenderer::initializeRootSignature() noexcept {
	getLogger().trace(L"Initializing root signature");
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];   // 根签名Slot列表。在着色器中可以通过slot+index获取到信息，信息储存在描述符堆中。
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {0}; // 采样器描述符

	const CD3DX12_DESCRIPTOR_RANGE range = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
	slotRootParameter[0].InitAsConstantBufferView(0);      // Slot 0: CBV类型，传递相机矩阵常量
	slotRootParameter[1].InitAsDescriptorTable(1, &range); // Slot 1: SRV类型，用于传递纹理 TODO(EmsiaetKadosh): Deprecated ?
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc {2, slotRootParameter, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}; // 用上述二者生成根签名描述符

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = 0;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
	if (errorBlob != nullptr) getLogger().error(u8to16w_nothrow(static_cast<char*>(errorBlob->GetBufferPointer())));
	ifFailed(hr, L"Failed to serialize D3D12 root signature."); // 序列化
	hr = device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	ifFailed(hr, L"Failed to create D3D12 root signature"); // 最终创建
	return false;
}

bool DirectRenderer::initializePipelineState() noexcept {
	getLogger().trace(L"Initializing pipeline state");
	HRESULT result = 0;
	// 填充PSO描述结构
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(coloredVertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(coloredPixelShader.Get());
	psoDesc.InputLayout = {COLORED_LAYOUT, COLORED_LAYOUT_SIZE};
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.RenderTarget[0].BlendEnable = false;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.SampleMask = UINT_MAX;
	result = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&coloredPipelineState));
	ifFailed(result, L"Failed to create graphics pipeline 1");
	// 第二结构
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc2 = {};
	psoDesc2.pRootSignature = rootSignature.Get();
	psoDesc2.VS = CD3DX12_SHADER_BYTECODE(textureVertexShader.Get());
	psoDesc2.PS = CD3DX12_SHADER_BYTECODE(texturePixelShader.Get());
	psoDesc2.InputLayout = {TEXTURE_LAYOUT, TEXTURE_LAYOUT_SIZE};
	psoDesc2.NumRenderTargets = 1;
	psoDesc2.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc2.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc2.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc2.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc2.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc2.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc2.SampleDesc.Count = 1;
	psoDesc2.SampleDesc.Quality = 0;
	psoDesc2.SampleMask = UINT_MAX;
	result = device->CreateGraphicsPipelineState(&psoDesc2, IID_PPV_ARGS(&texturePipelineState));
	ifFailed(result, L"Failed to create graphics pipeline 2");
	return false;
}

bool DirectRenderer::initializeFrames() noexcept {
	getLogger().trace(L"Initializing frames");
	try {
		frames.clear();
		frames.resize(swapFrameCount, DirectFrame(*this));
	}
	catch (std::bad_alloc&) {
		getLogger().error(L"Failed to resize DirectFrame: BadAlloc");
		game.getRiskManager().report();
	}
	catch (std::exception& e) {
		getLogger().ofNoexcept(L"Failed to initialize DirectFrame: ", u8to16w_nothrow(e.what())).error();
		game.getRiskManager().report();
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	for (unsigned int i = 0; i < swapFrameCount; ++i) if (frames[i].initialize(i, rtvHandle, dsvHandle)) return true;
	return false;
}

void DirectRenderer::finalizeFrames() noexcept { for (auto& frame: frames) frame.finalize(); }

void DirectRenderer::finalizePipelineState() noexcept {
	HBP_DX_RESET(coloredPipelineState);
	HBP_DX_RESET(texturePipelineState);
}

void DirectRenderer::finalizeRootSignature() noexcept { HBP_DX_RESET(rootSignature); }

void DirectRenderer::finalizeShader() noexcept {
	HBP_DX_RESET(coloredVertexShader);
	HBP_DX_RESET(coloredPixelShader);
	HBP_DX_RESET(textureVertexShader);
	HBP_DX_RESET(texturePixelShader);
}

void DirectRenderer::finalizeFence() noexcept { HBP_DX_RESET(fence); }

void DirectRenderer::finalizeHeap() noexcept {
	HBP_DX_RESET(rtvHeap);
	HBP_DX_RESET(dsvHeap);
}

void DirectRenderer::finalizeSwapChain() noexcept {
	if (swapChain) {
		BOOL fullScreen = false;
		HRESULT result = 0;
		result = swapChain->GetFullscreenState(&fullScreen, nullptr);
		if (fullScreen || FAILED(result))
			DiscardReturn(swapChain->SetFullscreenState(false, nullptr));
		swapChain.Reset();
	}
}

void DirectRenderer::finalizeCommandQueue() noexcept { HBP_DX_RESET(commandQueue); }
void DirectRenderer::finalizeDevice() noexcept { HBP_DX_RESET(device); }
void DirectRenderer::finalizeFactory() noexcept { HBP_DX_RESET(factory); }

unsigned long long DirectRenderer::requestSignal() noexcept {
	const unsigned long long value = ++this->fenceValue;
	HRESULT result = 0;
	result = commandQueue->Signal(fence.Get(), value);
	if (FAILED(result)) {
		getLogger().ofNoexcept(L"Failed to request signal. Error: ", getError(result)).error();
		game.crash(L"Failed to request signal.");
	}
	return value;
}

void DirectRenderer::awaitSignal(unsigned long long awaitingFenceValue) noexcept {
	if (fence->GetCompletedValue() < awaitingFenceValue) {
		HRESULT result = 0;
		result = fence->SetEventOnCompletion(awaitingFenceValue, fenceEvent);
		if (FAILED(result)) {
			getLogger().ofNoexcept(L"Failed to wait fenceValue ", awaitingFenceValue, L". Error: ", getError(result)).error();
			game.crash(L"Failed to wait fenceValue " + std::to_wstring(awaitingFenceValue) + L".");
		}
		else WaitForSingleObject(fenceEvent, INFINITE);
	}
}

DirectFrame& DirectRenderer::switchNextFrame() noexcept {
	++currentFrame %= swapFrameCount;
	return frames[currentFrame];
}

bool DirectRenderer::isTerminating() const noexcept { return rendererState & Terminating; }
bool DirectRenderer::isReady() const noexcept { return rendererState & Ready && !isTerminating(); }
bool DirectRenderer::isRendering() const noexcept { return rendererState & Rendering; }
bool DirectRenderer::builtinResize() noexcept { return false; }
DirectRenderer::DirectRenderer(const unsigned int common, const unsigned int spec) noexcept : rendererLogger(MainLoggerRouter.ofDomain(L"\\#FFD0E9FC" "DxRender")), textureManager(*this), resourceManager(*this, common, spec, &textureManager) {}

bool DirectRenderer::initialize(const HWND hwnd) noexcept {
	if (rendererState != Uninitialized) return false;
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = width;
	scissorRect.bottom = height;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	this->getError(0); // trigger init
	return
		initializeFactory() ||
		initializeDevice(true) ||
		initializeCommandQueue() ||
		initializeSwapChain(hwnd) ||
		initializeHeap() ||
		initializeFence() ||
		initializeShader() ||
		initializeRootSignature() ||
		initializePipelineState() ||
		initializeFrames() ||
		// (debugDefault(), false) ||
		(rendererState = Ready, false);
}

void DirectRenderer::finalize() noexcept {
	// getLogger().warn(L"Renderer terminating");
	rendererState |= Terminating;
	while (isRendering()) _mm_pause();
	if (commandQueue) awaitAllFrames();
	else getLogger().error(L"Command queue not available.");
	finalizeFrames();
	finalizePipelineState();
	finalizeRootSignature();
	finalizeShader();
	finalizeFence();
	finalizeHeap();
	finalizeSwapChain();
	finalizeCommandQueue();
	finalizeDevice();
	finalizeFactory();
	debugDefault();
}

void DirectRenderer::awaitAllFrames() noexcept { for (DirectFrame& frame: frames) frame.awaitFrame(); }

void DirectRenderer::debugDefault() noexcept {
	HRESULT hr = 0;
	if (!device) getLogger().ofNoexcept(L"Device not available.", FunctionSignature()).warn();
	else if (ComPtr<ID3D12InfoQueue> infoQueue; FAILED(device.As(&infoQueue))) getLogger().error(L"device as infoQueue failed");
	else if (const UINT64 messageCount = infoQueue->GetNumStoredMessages()) {
		for (UINT64 i = 0; i < messageCount; i++) {
			SIZE_T messageLength = 0;                                // 获取消息大小
			hr = infoQueue->GetMessageW(i, nullptr, &messageLength); // 第一次调用获取长度
			if (FAILED(hr)) {
				getLogger().error(L"Failed to get message length");
				break;
			}
			D3D12_MESSAGE* message = static_cast<D3D12_MESSAGE*>(malloc(messageLength)); // 分配内存并读取消息
			hr = infoQueue->GetMessageW(i, message, &messageLength);
			if (FAILED(hr)) getLogger().error(L"Failed to get message content");
			else {
				using logger = DomainLogger& (DomainLogger::*)(RenderableString&& msg) noexcept;
				static constexpr logger members[] = {
					&DomainLogger::error,
					&DomainLogger::error,
					&DomainLogger::warn,
					&DomainLogger::info,
					&DomainLogger::log
				};
				(getLogger().*members[message->Severity])(L"[D3D12] " + u8to16w_nothrow(message->pDescription)); // 输出消息内容
			}
			free(message);
		}
		infoQueue->ClearStoredMessages(); // 清空消息队列（避免重复处理）
	}
	else getLogger().debug(L"No dx12 debug info.");

	ComPtr<IDXGIDebug1> dxgiDebug;
	if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) getLogger().ofNoexcept(L"Failed to get debug interface").warn();
	else {
		// DXGI_DEBUG_RLO_SUMMARY: 输出摘要信息
		// DXGI_DEBUG_RLO_DETAIL: 输出详细信息
		// DXGI_DEBUG_RLO_IGNORE_INTERNAL: 忽略内部引用（非常重要，避免误报）
		hr = dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		if (FAILED(hr)) getLogger().ofNoexcept(L"Failed to report live objects. Error: ", hr).warn();
		else getLogger().ofNoexcept(L"Successfully reported live objects.").debug();
	}
}

void DirectRenderer::debugCustom() noexcept {
	getLogger().ofNoexcept(L"fenceValue = ", fenceValue).debug();
	getLogger().ofNoexcept(L"rtvDescriptorSize = ", rtvDescriptorSize).debug();
	getLogger().ofNoexcept(L"swapFrameCount = ", swapFrameCount).debug();
	getLogger().ofNoexcept(L"frameIndex = ", currentFrame).debug();
	getLogger().ofNoexcept(L"size = ", getWidth(), L", ", getHeight()).debug();
	getLogger().ofNoexcept(L"factory @ ", ptrtow(factory.Get())).debug();
	getLogger().ofNoexcept(L"device @ ", ptrtow(device.Get())).debug();
	getLogger().ofNoexcept(L"rootSignature @ ", ptrtow(rootSignature.Get())).debug();
	getLogger().ofNoexcept(L"coloredPipelineState @ ", ptrtow(coloredPipelineState.Get())).debug();
	getLogger().ofNoexcept(L"texturePipelineState @ ", ptrtow(texturePipelineState.Get())).debug();
	getLogger().ofNoexcept(L"coloredVertexShader @ ", ptrtow(coloredVertexShader.Get())).debug();
	getLogger().ofNoexcept(L"coloredPixelShader @ ", ptrtow(coloredPixelShader.Get())).debug();
	getLogger().ofNoexcept(L"textureVertexShader @ ", ptrtow(textureVertexShader.Get())).debug();
	getLogger().ofNoexcept(L"texturePixelShader @ ", ptrtow(texturePixelShader.Get())).debug();
	getLogger().ofNoexcept(L"rtvHeap @ ", ptrtow(rtvHeap.Get())).debug();
	getLogger().ofNoexcept(L"commandQueue @ ", ptrtow(commandQueue.Get())).debug();
	getLogger().ofNoexcept(L"swapChain @ ", ptrtow(swapChain.Get())).debug();
	getLogger().ofNoexcept(L"fence @ ", ptrtow(fence.Get())).debug();
	getLogger().ofNoexcept(L"viewport = W:", viewport.TopLeftX, L'+', viewport.Width, L", H:", viewport.TopLeftY, L'+', viewport.Height, L", D:", viewport.MinDepth, L'~', viewport.MaxDepth).debug();
	// scissorRect
	getLogger().ofNoexcept(L"fenceEvent @ ", ptrtow(fenceEvent)).debug();
	for (unsigned int i = 0; i < frames.size(); ++i) frames[i].debugCustom(i);
}

void DirectRenderer::declareThread() noexcept(false) {
	if (threadUseState) throw ThreadInterferenceException((std::wostringstream() << L"Thread has been declared by id: " << idThread).str());
	threadUseState = Declared;
	idThread = std::this_thread::get_id();
}

void DirectRenderer::assertThread() const noexcept(false) {
	if (!threadUseState) throw ThreadInterferenceException((std::wostringstream() << L"Thread is not declared").str());
	if (std::this_thread::get_id() != idThread) throw ThreadInterferenceException((std::wostringstream() << L"Thread id should be " << idThread).str());
}

bool DirectRenderer::checkThread() const noexcept(false) { return !threadUseState || std::this_thread::get_id() != idThread; }

// ReSharper disable once CppMemberFunctionMayBeStatic
String DirectRenderer::getError(const HRESULT hresult) noexcept {
	try {
		static Map<HRESULT, String> errors = [] {
			Map<HRESULT, String> ret;
			initializeReturnCode(ret);
			return ret;
		}();
		const auto iter = errors.find(hresult);
		if (iter == errors.cend()) return qwtowb16_nothrow(hresult, 8);
		return iter->second;
	}
	catch (...) { return L"? (Exception in getError)"; }
}

void DirectRenderer::begin() {
	rendererState |= Rendering;
	if (!isReady()) {
		rendererState &= ~Rendering;
		return;
	}
	assertThread();
	bool flag = false;
	scissorRect.left = 0;
	scissorRect.top = 0;
	if (scissorRect.right != width) viewport.Width = static_cast<float>(scissorRect.right = width), flag = true;
	if (scissorRect.bottom != height) viewport.Height = static_cast<float>(scissorRect.bottom = height), flag = true;
	if (flag) builtinResize();
	switchNextFrame().begin();
}

void DirectRenderer::end() {
	if (isRendering()) {
		assertThread();
		frames[currentFrame].end();
	}
	rendererState &= ~Rendering;
}

void DirectRenderer::changeScene(const DirectTextureScene& scene) noexcept { resourceManager.switchScene(DirectTextureScene(scene)); }
void DirectRenderer::changeScene(DirectTextureScene&& scene) noexcept { resourceManager.switchScene(std::move(scene)); }

void DirectRenderer::drawTexture(const DirectTextureContext&) { throw NotImplementedException(u8to16w_nothrow(__FUNCSIG__)); }

void DirectRenderer::drawColor(ColoredSet&& set) noexcept(false) {
	if (!isRendering()) return;
	assertThread();
	frames[currentFrame].drawColor(std::move(set));
}

void DirectRenderer::drawColor(const ColoredSet& set) noexcept(false) {
	if (!isRendering()) return;
	assertThread();
	frames[currentFrame].drawColor(set);
}

inline unsigned int DirectRenderer::getWidth() const noexcept { return scissorRect.right; }
inline unsigned int DirectRenderer::getHeight() const noexcept { return scissorRect.bottom; }
inline unsigned int DirectRenderer::getUpdatedWidth() const noexcept { return width; }
inline unsigned int DirectRenderer::getUpdatedHeight() const noexcept { return height; }

inline void DirectTextureManager::registerDefaultLoaders() noexcept {
	registerLoader<TextureLoaderBMP>(L"bmp");
}
