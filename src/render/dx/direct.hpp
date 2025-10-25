#pragma once

#define HBP_DX_DEBUG true

#include <thread>
#include <directx\d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "src\using.hpp"
#include "src\utils\math.hpp"
#include "def.hpp"
#include "texture.hpp"

using CameraMatrix = DirectX::XMFLOAT4X4;

class DirectCamera {
	Vector3D position = Vector3D(0, 0, 0);
	Vector3D direction = Vector3D(0, 0, 1);
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
	void setDirection(const double pitch, const double yaw) noexcept { changed = true, this->pitch = pitch, this->yaw = yaw, calculatePitchYaw(); }
	void setDirection(const double x, const double y, const double z) noexcept { changed = true, direction.setX(x), direction.setY(y), direction.setZ(z), direction.lengthManhattan() == 0 ? direction.setX(0) : direction.normalize(); }
	void setPosition(const Vector3D& p) noexcept { changed = true, position = p; }
	void setPosition(const double x, const double y, const double z) noexcept { changed = true, position.setX(x), position.setY(y), position.setZ(z); }
	void move(const Vector3D& r) noexcept { changed = true, position += r; }
	[[nodiscard]] double getCenterDistance() const noexcept { return distance; }
	[[nodiscard]] double getFoV() const noexcept { return fieldOfView; }
	[[nodiscard]] double getFarthestDistance() const noexcept { return farthestDistance; }
	[[nodiscard]] double getNearestDistance() const noexcept { return nearestDistance; }
	void setCenterDistance(const double distance) noexcept { changed = true, this->distance = distance; }
	void setFoV(const double fov) noexcept { changed = true, this->fieldOfView = fov; }
	void setFarthestDistance(const double fd) noexcept { changed = true, this->farthestDistance = fd; }
	void setNearestDistance(const double nd) noexcept { changed = true, this->nearestDistance = nd; }
	void calculate(double width, double height) noexcept;
	void rotate(const double pitch, const double yaw) noexcept { changed = true, this->pitch += pitch, this->yaw += yaw, calculatePitchYaw(); }

	void usePerspectiveProjection() noexcept { changed = true, projection = Perspective; }
	void useOrthographicProjection() noexcept { changed = true, projection = Orthographic; }
	void useProjectionSwitch() noexcept { changed = true, projection = projection == Perspective ? Orthographic : Perspective; }
	bool consumeChanges() noexcept { return changed ? changed = false, true : false; }
	void copy(char* buffer) const noexcept { memcpy(buffer, &conMatrix, sizeof(conMatrix)); }
};

struct DirectResource {
	ComPtr<ID3D12Resource> buffer = {};
	size_t currentSize = 1;

	enum Status {
		Null,
		Map,
		Unmap
	} status = Null;

	[[nodiscard]] bool resize(DirectRenderer& renderer, unsigned long long& amount, size_t unitSize) noexcept;
	[[nodiscard]] bool resize(DirectRenderer& renderer, size_t unitSize) noexcept;
	[[nodiscard]] char* map(DirectRenderer& renderer) noexcept;
	void unmap() noexcept;
	void finalize() noexcept;
	[[nodiscard]] ID3D12Resource* getBuffer() const noexcept;
};

/**
 * 纹理资源缓冲区
 */
struct DirectTextureResource {
	using DirectTextureIndex::BufferType;
	Vector<DirectTextureEntry> textures;
	ComPtr<ID3D12Resource> buffer = {};
	unsigned int unitWidth, unitHeight;
	BufferType type;

	enum Status : unsigned char {
		Await,
		Ready,
		Uploaded
	} status = Await;

	DirectTextureResource(const BufferType type, const unsigned int unitWidth, const unsigned int unitHeight) : unitWidth(unitWidth), unitHeight(unitHeight), type(type) {}

	/**
	 * 绑定纹理到当前缓冲区
	 * @param texture 要绑定的资源对象
	 * @return 绑定后的index值
	 */
	DirectTextureIndex bindTexture(DirectTextureCarrier& texture) noexcept {
		if (!texture.isReady()) return -1;
		if (texture.index) return texture.index;
		if (texture.type == DirectTextureCarrier::Sprite) texture
		return texture.getIndex();
	}

	void upload() noexcept {}
	[[nodiscard]] bool isFull() const noexcept { return textures.size() >= 200; }
	[[nodiscard]] bool isSuitable(const DirectTextureCarrier& carrier) const noexcept { return carrier.width <= unitWidth && carrier.height <= unitHeight; }
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
		Accept,
		Upload,
		Process,
		Completed,
		Reset,
		Await,
	} state = Await;

	DirectFrame(DirectRenderer& renderer) : renderer(renderer) {}
	bool reassignColoredBuffer(size_t size) noexcept;
	bool reassignTextureBuffer(size_t size) noexcept;
	bool reassignColoredIndexBuffer(size_t size) noexcept;
	bool reassignDepthStencil() noexcept;
	bool initialize(unsigned int index, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& dsvHandle);
	void awaitFrame() noexcept;
	void finalize() noexcept;
	void debugCustom(unsigned int index) noexcept;

	void begin() noexcept(false);
	bool submitVertices() noexcept;
	void end();
	void assertStatus(State, bool strict = false) const noexcept(false);

	void drawColor(ColoredSet&&) noexcept(false);
	void drawColor(const ColoredSet&) noexcept(false);

	[[nodiscard]] unsigned int getBufferSizeAT() const noexcept;
};

class DirectRenderer {
	friend struct DirectFrame;
	friend struct DirectResource;
	friend struct DirectConstantResource;
	DirectTextureManager textureManager;
	DirectCamera camera;

	std::thread::id idThread;
	unsigned long long fenceValue = 0;
	unsigned int rtvDescriptorSize = 0;
	unsigned int dsvDescriptorSize = 0;
	unsigned int swapFrameCount = 2;
	unsigned int currentFrame = swapFrameCount - 1;
	int width = 1920, height = 1080;

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

	enum SpritePreset : unsigned char {
		S256x256,
		S512x512,
		S1024x1024,
		S2048x1024,
		S4096x2048,
	};

	SpritePreset spritePreset = S256x256;
	ThreadUseState threadUseState = NotDeclared;
	unsigned char rendererState = Uninitialized;

	ComPtr<IDXGIFactory6> factory = nullptr;
	ComPtr<ID3D12Device6> device = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	ComPtr<ID3D12PipelineState> coloredPipelineState = nullptr;
	ComPtr<ID3D12PipelineState> texturePipelineState = nullptr;
	ComPtr<ID3D12Resource> textureBuffer = nullptr;
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
	void awaitSignal(unsigned long long fenceValue) noexcept;
	DirectFrame& switchNextFrame() noexcept;
	[[nodiscard]] bool isTerminating() const noexcept;
	[[nodiscard]] bool isReady() const noexcept;
	[[nodiscard]] bool isRendering() const noexcept;
	bool builtinResize() noexcept;

public:
	/**
	 * @return true - initialization failed
	 */
	bool initialize(HWND hwnd) noexcept;
	void finalize() noexcept;
	void debugDefault() noexcept;
	void debugCustom() noexcept;
	void declareThread() noexcept(false);
	void assertThread() const noexcept(false);
	String getError(HRESULT hresult) noexcept;
	DirectCamera& getCamera() noexcept { return camera; }
	void setViewport(const int width, const int height) noexcept { this->width = width, this->height = height; }

	void begin();
	void end();

	void uploadTexture(Texture&);
	void drawTexture(const DirectTextureContext&);
	void drawColor(ColoredSet&&) noexcept(false);
	void drawColor(const ColoredSet&) noexcept(false);

	[[nodiscard]] unsigned int getWidth() const noexcept;
	[[nodiscard]] unsigned int getHeight() const noexcept;
	[[nodiscard]] unsigned int getUpdatedWidth() const noexcept;
	[[nodiscard]] unsigned int getUpdatedHeight() const noexcept;

	[[nodiscard]] unsigned int getBufferSizeCP() const noexcept;
};
