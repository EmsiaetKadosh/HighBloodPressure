#pragma once

#define HBP_DX_DEBUG true
/**
 * 宏HBP_DX_IMPL指定是否在后续的文件中包含DX相关的接口文件。
 * 如果定义HBP_DX_IMPL，则需要提供依赖DX接口文件的所有的定义（和必要实现）
 * 如果未定义HBP_DX_IMPL，则可以隐藏不对外暴露的依赖DX接口文件的定义，只保留声明。
 */
#define HBP_DX_IMPL

#include <unordered_map>
#include <thread>
#include <directx\d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "src\utils\math.hpp"
#include "src\utils\logger.hpp"
#include "def.hpp"
#include "texture.hpp"

using CameraMatrix = DirectX::XMFLOAT4X4;

class DirectCamera {
	Vector3D position = Vector3D(10, 10, 10);
	Vector3D direction = Vector3D(1, -1, -1).normalize();
	CameraMatrix conMatrix;
	double distance; // 相机与相机绕转中心
	double nearestDistance = 0.1; // 近距
	double farthestDistance = 100; // 远距
	double fieldOfView = 1.3; // 视场角弧度
	double pitch = 0.0, yaw = 0.0;
	bool changed = true; // 1

	enum : unsigned char {
		Perspective = 0,
		Orthographic = 1,
	} projection = Perspective; // 1


	void calculatePitchYaw() noexcept;

public:
	DirectCamera() = default;
	[[nodiscard]] Vector3D getDirection() const noexcept { return direction; }
	[[nodiscard]] Vector3D getPosition() const noexcept { return position; }
	void setDirection(const Vector3D& d) noexcept { changed = true, direction = d; }
	void setDirection(const double pitch_, const double yaw_) noexcept { changed = true, pitch = pitch_, yaw = yaw_, calculatePitchYaw(); }
	void setDirection(const double x, const double y, const double z) noexcept { changed = true, direction.setX(x), direction.setY(y), direction.setZ(z), direction.lengthManhattan() == 0 ? direction.setX(0) : direction.normalize(); }
	void setPosition(const Vector3D& p) noexcept { changed = true, position = p; }
	void setPosition(const double x, const double y, const double z) noexcept { changed = true, position.setX(x), position.setY(y), position.setZ(z); }
	void move(const Vector3D& r) noexcept { changed = true, position += r; }
	[[nodiscard]] double getCenterDistance() const noexcept { return distance; }
	[[nodiscard]] double getFoV() const noexcept { return fieldOfView; }
	[[nodiscard]] double getFarthestDistance() const noexcept { return farthestDistance; }
	[[nodiscard]] double getNearestDistance() const noexcept { return nearestDistance; }
	void setCenterDistance(const double dist) noexcept { changed = true, this->distance = dist; }
	void setFoV(const double fov) noexcept { changed = true, this->fieldOfView = fov; }
	void setFarthestDistance(const double fd) noexcept { changed = true, this->farthestDistance = fd; }
	void setNearestDistance(const double nd) noexcept { changed = true, this->nearestDistance = nd; }
	void calculate(double width, double height) noexcept;
	void rotate(const double pitch_, const double yaw_) noexcept { changed = true, pitch += pitch_, yaw += yaw_, calculatePitchYaw(); }

	void usePerspectiveProjection() noexcept { changed = true, projection = Perspective; }
	void useOrthographicProjection() noexcept { changed = true, projection = Orthographic; }
	void useProjectionSwitch() noexcept { changed = true, projection = projection == Perspective ? Orthographic : Perspective; }
	bool consumeChanges() noexcept { return changed ? changed = false, true : false; }
	void copy(char* buffer) const noexcept { memcpy(buffer, &conMatrix, sizeof(conMatrix)); }
};

struct DirectResource {
	enum class DirectResourceStatus : unsigned char {
		Null,
		Map,
		Unmap
	};

	DirectRenderer& renderer;
	ComPtr<ID3D12Resource> buffer = {};
	size_t currentSize = 1;
	DirectResourceStatus status = DirectResourceStatus::Null; // 1B

	DirectResource(DirectRenderer& renderer) noexcept : renderer(renderer) {}

	[[nodiscard]] bool resize(unsigned long long& amount, size_t unitSize) noexcept;
	[[nodiscard]] bool resize(size_t unitSize) noexcept;
	[[nodiscard]] char* map() noexcept;
	void unmap() noexcept;
	void finalize() noexcept;
	[[nodiscard]] ID3D12Resource* getBuffer() const noexcept;
};

/**
 * 纹理资源缓冲区；
 * CB PB AB TB都是DirectTextureResource
 */
struct DirectTextureResource {
private:
	friend class DirectFrame;
	friend class DirectTextureResourceManager;
	using DirectResourceStatus = DirectResource::DirectResourceStatus;
	DirectRenderer& renderer;
	ComPtr<ID3D12Resource> uploadBuffer = {}; // CPU write
	ComPtr<ID3D12Resource> defaultBuffer = {}; // GPU only
	char* mapAddress = nullptr;
	size_t size = 1;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	UINT numRows = 0;
	UINT64 rowPitch = 0;
	UINT64 uploadBufferSize = 0;
	unsigned int width = 0, height = 0;
	/**
	 * @brief 此变量用于提高效率。
	 * 每次Scene中的内容发生更改时，其版本值+1。
	 * 渲染提交时，查看DTS中的版本值与此值是否相同，不同则重新打包上传。
	 * 参考@code DirectTextureScene::version	@endcode。
	 */
	unsigned int version = 0;
	DirectResourceStatus status = DirectResourceStatus::Null;
	[[nodiscard]] char* map() noexcept;
	void unmap() noexcept;
	void submit(const ComPtr<ID3D12GraphicsCommandList>& commandList) const noexcept;

public:
	DirectTextureResource(DirectRenderer& renderer) noexcept : renderer(renderer) {}

	[[nodiscard]] bool isSuitable(const DirectTextureCarrier& carrier) const noexcept { return carrier.width <= width && carrier.height <= height; }
	[[nodiscard]] bool isOutdated(const unsigned int v) const noexcept { return v != version; }
	[[nodiscard]] bool resize(unsigned int singleWidth, unsigned int singleHeight, unsigned int length) noexcept;
	void finalize() noexcept;
	[[nodiscard]] unsigned int getSize() const noexcept { return size; }
	[[nodiscard]] char* getMappedAddress() const noexcept { return mapAddress; }

	/**
	 * @brief 填充纹理的部分区域
	 * @param index 纹理数组索引
	 * @param x 起始X坐标
	 * @param y 起始Y坐标
	 * @param w 宽度
	 * @param h 高度
	 * @param data 像素数据（格式为0xAARRGGBB，大小为w*h）
	 */
	bool fillRect(unsigned int index, unsigned int x, unsigned int y, unsigned int w, unsigned int h, const unsigned int* data) const noexcept;
};

class DirectTextureDispatcher {
	friend class DirectTextureResourceManager;

	enum Category : unsigned char { Unit, Special };

	struct Atlas {
		Vector<bool> grid; // 网格占用状态
		const Category category;
		unsigned int usedCells = 0; // 已使用分片数
		float fillRate = 0.0f;

		Atlas(const Category category, const unsigned int gridSize) : grid(gridSize * gridSize, false), category(category) {}
	};

	Vector<Atlas> atlases;
	HashMap<DirectTextureCarrier*, DirectTextureIndex> indices = {};
	DirectTextureResourceManager& manager;
	DirectRenderer& renderer;
	const DirectResourceBufferType buffer;

	DirectTextureDispatcher(const DirectResourceBufferType type, DirectTextureResourceManager& manager, DirectRenderer& renderer) noexcept : manager(manager), renderer(renderer), buffer(type) {}
	void setTrue(const unsigned int index, const unsigned int gridWidth, const unsigned int x, const unsigned y) noexcept { atlases[index].grid.at(x + y * gridWidth) = true; }
	void setFalse(const unsigned int index, const unsigned int gridWidth, const unsigned int x, const unsigned y) noexcept { atlases[index].grid.at(x + y * gridWidth) = false; }

	void setRange(const DirectTextureIndex& index, const bool value) noexcept {
		Vector<bool>& grids = atlases[index.index].grid;
		const unsigned int yEnd = index.yGridSize + index.yGrid;
		for (unsigned int i = index.yGrid; i < yEnd; ++i) std::fill_n(grids.begin() + index.xGrid, index.xGridSize, value);
	}

	static void updateAtlasFillRate(Atlas& atlas) noexcept;
	void createAtlas(Category category) noexcept;
	[[nodiscard]] Category categoryOf(const DirectTextureCarrier* carrier) const noexcept;
	[[nodiscard]] std::pair<unsigned int, unsigned int> indexToXY(unsigned int index) const noexcept;
	[[nodiscard]] bool isLineEmpty(Atlas& atlas, unsigned int x, unsigned int y, unsigned int width /* NonZero. 1~n */) const noexcept;
	[[nodiscard]] bool isColumnEmpty(Atlas& atlas, unsigned int x, unsigned int y, unsigned int height /* NonZero. 1~n */) const noexcept;

	[[nodiscard]] DirectTextureIndex newPrepare(const DirectTextureCarrier* carrier) const noexcept;
	void newAdapt(const DirectTextureCarrier* carrier, DirectTextureIndex& index) noexcept;

	bool fitForUnit(unsigned int indexAtlas, DirectTextureIndex& index, const DirectTextureCarrier* carrier) noexcept;
	bool fitForSpecial(unsigned int indexAtlas, DirectTextureIndex& index, const DirectTextureCarrier* carrier) noexcept;

public:
	void update() noexcept;
	void apply(DirectTextureResource& resource) noexcept;
	const DirectTextureIndex& getIndex(DirectTextureCarrier* carrier) noexcept;
};

class DirectTextureResourceManager {
	DirectTextureScene currentScene;
	HashSet<DirectTextureCarrier*> constant;
	DirectRenderer& renderer;
	DirectTextureDispatcher dispatcherConstant;
	DirectTextureDispatcher dispatcherPreload;
	DirectTextureDispatcher dispatcherAppend;
	DirectTextureDispatcher dispatcherTemporary;
	unsigned int unitSize;
	unsigned int specialSize;
	unsigned int edgeSize;
	unsigned int gridSize;
	float fillThreshold = 0.7;
	bool constantOk = false;

public:
	DirectTextureResourceManager(DirectRenderer& renderer, const unsigned int unit, const unsigned int special, DirectTextureManager* manager) noexcept :
		currentScene(manager), renderer(renderer),
		dispatcherConstant { DirectResourceBufferType::Constant, *this, renderer },
		dispatcherPreload { DirectResourceBufferType::Preload, *this, renderer },
		dispatcherAppend { DirectResourceBufferType::Append, *this, renderer },
		dispatcherTemporary { DirectResourceBufferType::Temporary, *this, renderer },
		unitSize(unit), specialSize(special), edgeSize(specialSize % unitSize), gridSize(specialSize / unitSize) {}

	void prepareConstant(DirectTextureResource& buffer) noexcept;
	void preparePreload(DirectTextureResource& buffer) noexcept;
	/** @returns true - 需要重新上传 */
	[[nodiscard]] bool prepareAppend(DirectTextureResource& buffer) noexcept;
	/** @returns true - 需要重新上传 */
	[[nodiscard]] bool prepareTemporary(DirectTextureResource& buffer) noexcept;
	/**
	 * @brief 向ConstantBuffer填入纹理
	 * @param carrier 要填入的纹理
	 * @return true - 成功
	 */
	bool addConstant(DirectTextureCarrier* const carrier) noexcept {
		if (constantOk) return false;
		if (constant.contains(carrier)) return true;
		constant.emplace(carrier);
		return true;
	}

	DirectTextureScene& addPreload(DirectTextureCarrier* const carrier) noexcept { return currentScene.addPreload(carrier); }
	DirectTextureScene& addAppend(DirectTextureCarrier* const carrier) noexcept { return currentScene.addAppend(carrier); }
	DirectTextureScene& addTemporary(DirectTextureCarrier* const carrier) noexcept { return currentScene.addTemporary(carrier); }
	void switchScene(DirectTextureScene&& newScene) noexcept { currentScene = std::move(newScene); }
	[[nodiscard]] DirectTextureScene& getCurrentScene() noexcept { return currentScene; }
	[[nodiscard]] unsigned int getUnitSize() const noexcept { return unitSize; }
	[[nodiscard]] unsigned int getSpecialSize() const noexcept { return specialSize; }
	[[nodiscard]] float getFillThreshold() const noexcept { return fillThreshold; }
	[[nodiscard]] unsigned int getGridSize() const noexcept { return gridSize; }
	[[nodiscard]] unsigned int getEdgeSize() const noexcept { return edgeSize; }
};

struct DirectFrame {
	Vector<ColoredSet> coloredBuffer;
	Vector<TextureSet> textureBuffer;
	DirectRenderer& renderer;

	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	DirectResource constants;
	DirectResource coloredVertex;
	DirectResource textureVertex;
	DirectResource coloredIndex;
	DirectTextureResource textureAppend;
	DirectTextureResource textureTemporary;
	ComPtr<ID3D12Resource> renderTargetView = nullptr;
	ComPtr<ID3D12Resource> depthStencilView = nullptr;
	unsigned long long thisSignal = 0;
	unsigned long long maxColoredIndexCount = 0;
	unsigned long long maxColoredVertexCount = 0;
	unsigned long long maxTextureVertexCount = 0;
	unsigned long long actualColoredIndexCount = 0;
	unsigned long long actualColoredVertexCount = 0;
	unsigned long long actualTextureVertexCount = 0;

	enum State : unsigned char {
		Ready, // 帧缓冲空闲，可开始录制
		Recording, // CPU 正在录制命令
		Executing, // GPU 正在执行
	} state = Ready;

	DirectFrame(DirectRenderer& renderer) noexcept :
		renderer(renderer), constants(renderer),
		coloredVertex(renderer), textureVertex(renderer), coloredIndex(renderer),
		textureAppend(renderer), textureTemporary(renderer) {}

	bool reassignColoredBuffer(size_t size) noexcept;
	bool reassignTextureBuffer(size_t size) noexcept;
	bool reassignColoredIndexBuffer(size_t size) noexcept;
	bool reassignDepthStencil() noexcept;
	bool initialize(unsigned int index, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& dsvHandle);
	void awaitFrame() noexcept;
	void finalize() noexcept;
	void debugCustom(unsigned int index) noexcept;

	void begin() noexcept(false);
	bool submitColoredVertices() noexcept;
	bool submitTextureVertices() noexcept;
	bool submitTextures() noexcept;
	void end();
	void assertRecording(bool strict = false) const noexcept(false);

	void drawColor(ColoredSet&&) noexcept(false);
	void drawColor(const ColoredSet&) noexcept(false);
};

class DirectRenderer {
	friend struct DirectFrame;
	friend struct DirectResource;
	friend struct DirectTextureResource;
	DomainLogger rendererLogger;
	DirectCamera camera = {};

	unsigned long long fenceValue = 0;
	unsigned int rtvDescriptorSize = 0;
	unsigned int dsvDescriptorSize = 0;
	unsigned int swapFrameCount = 2;
	unsigned int currentFrame = swapFrameCount - 1;
	int width = 1920, height = 1080;
	std::thread::id idThread; // 4B

	enum ThreadUseState : unsigned char {
		NotDeclared = 0,
		Declared,
	};

	enum RendererState : unsigned char {
		Uninitialized = 0,
		Ready = 1,
		Rendering = 2,
		Terminating = 128
	};

	ThreadUseState threadUseState = NotDeclared; // 1B
	unsigned char rendererState = Uninitialized; // 1B
	DirectTextureManager textureManager;
	DirectTextureResourceManager resourceManager;

	ComPtr<IDXGIFactory6> factory = nullptr;
	ComPtr<ID3D12Device6> device = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	ComPtr<ID3D12PipelineState> coloredPipelineState = nullptr;
	ComPtr<ID3D12PipelineState> texturePipelineState = nullptr;
	DirectTextureResource textureConstant = *this;
	DirectTextureResource texturePreload = *this;
	ComPtr<ID3DBlob> coloredVertexShader = nullptr;
	ComPtr<ID3DBlob> coloredPixelShader = nullptr;
	ComPtr<ID3DBlob> textureVertexShader = nullptr;
	ComPtr<ID3DBlob> texturePixelShader = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV-heap
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<IDXGISwapChain3> swapChain = nullptr;
	ComPtr<ID3D12Fence> fence = nullptr;
	Vector<DirectFrame> frames;
	D3D12_VIEWPORT viewport = {
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width = 800.0f,
		.Height = 600.0f,
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f
	};
	D3D12_RECT scissorRect = {};
	HANDLE fenceEvent = nullptr;

	bool initializeFactory() noexcept;
	bool initializeDevice(bool requestHighPerformance) noexcept;
	bool initializeCommandQueue() noexcept;
	bool initializeSwapChain(HWND hwnd) noexcept;
	bool initializeHeap() noexcept;
	bool initializeFence() noexcept;
	bool initializeShader() noexcept;
	bool initializeRootSignature() noexcept;
	bool initializePipelineState() noexcept;
	bool initializeFrames() noexcept; // Command, Buffer

	void finalizeFrames() noexcept; // Command, Buffer
	void finalizePipelineState() noexcept;
	void finalizeRootSignature() noexcept;
	void finalizeShader() noexcept;
	void finalizeFence() noexcept;
	void finalizeHeap() noexcept;
	void finalizeSwapChain() noexcept;
	void finalizeCommandQueue() noexcept;
	void finalizeDevice() noexcept;
	void finalizeFactory() noexcept;

	unsigned long long requestSignal() noexcept;
	void awaitSignal(unsigned long long awaitingFenceValue) noexcept;
	DirectFrame& switchNextFrame() noexcept;
	[[nodiscard]] bool isTerminating() const noexcept;
	[[nodiscard]] bool isReady() const noexcept;
	[[nodiscard]] bool isRendering() const noexcept;
	bool builtinResize() noexcept;

public:
	/**
	 * @attention 如果遇到common尺寸的纹理，则会放在一起拼成spec尺寸的sprite；否则，尺寸不同的纹理会作为单独的spec尺寸的纹理被上传。
	 * @brief 创建纹理管理器
	 * @param common 常用纹理的尺寸
	 * @param spec 所有纹理中的最大尺寸
	 */
	explicit DirectRenderer(unsigned int common = 16, unsigned int spec = 1024) noexcept;

	/**
	 * @return true - initialization failed
	 */
	bool initialize(HWND hwnd) noexcept;
	void finalize() noexcept;
	void awaitAllFrames() noexcept;
	void debugDefault() noexcept;
	void debugCustom() noexcept;
	void declareThread() noexcept(false);
	void assertThread() const noexcept(false);
	[[nodiscard]] bool checkThread() const noexcept(false);
	String getError(HRESULT hresult) noexcept;
	DomainLogger& getLogger() noexcept { return rendererLogger; }
	DirectCamera& getCamera() noexcept { return camera; }
	void setViewport(const int w, const int h) noexcept { width = w, height = h; }

	void begin();
	void end();

	void changeScene(const DirectTextureScene& scene) noexcept;
	void changeScene(DirectTextureScene&& scene) noexcept;

	void drawTexture(const DirectTextureContext&);
	void drawColor(ColoredSet&&) noexcept(false);
	void drawColor(const ColoredSet&) noexcept(false);

	[[nodiscard]] unsigned int getWidth() const noexcept;
	[[nodiscard]] unsigned int getHeight() const noexcept;
	[[nodiscard]] unsigned int getUpdatedWidth() const noexcept;
	[[nodiscard]] unsigned int getUpdatedHeight() const noexcept;
};
