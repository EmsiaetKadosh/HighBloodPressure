
#pragma once

#include <list>
#include <map>
#include <utility>

#include "src\using.hpp"

class DirectTextureCarrier;
class DirectTexture;
class DirectTextureEntry;
class DirectTextureLoader;
class DirectTextureLoaderEntry;
class DirectTextureManager;

class DirectTextureCarrier {
	friend class DirectTextureLoader;
	friend class DirectTexture;
	friend struct DirectTextureResource;
	String file; // align 8
	DirectTextureLoader* loader = nullptr; // 8
	void* extraData = nullptr;
	unsigned int width = 0, height = 0; // 4 + 4
	unsigned int pack = 0; // 4
	DirectTextureIndex index; // 4

	enum Status : unsigned char {
		Register,
		Ready,
		Uploaded
	} status = Register; // 1

	enum PrepareType {
		Unknown,
		Resource,
		Sprite,
	} type = Unknown;

	DirectTextureCarrier(const unsigned int idPack, String&& file) : file(std::move(file)), pack(idPack) {}
	DirectTextureCarrier(const unsigned int idPack, String&& file, DirectTextureLoader* loader) : file(std::move(file)), loader(loader), pack(idPack) {}
	/**
	 * @param manager 纹理管理器
	 * @return true - 准备完毕，可以使用
	 */
	[[nodiscard]] bool load(DirectTextureManager* manager) noexcept;
	void unload() noexcept;
	void upload() noexcept;
	[[nodiscard]] const String& getFile() const noexcept { return file; }
	[[nodiscard]] bool isReady() const noexcept { return status == Ready || status == Uploaded; }
};

class DirectTexture {
	List<DirectTextureCarrier> carriers;
	DirectTextureManager* manager;
	DirectTextureCarrier* preparing = nullptr;

	/**
	 * @brief 获取最尾端的可用携带器
	 * @return true - 失败
	 */
	[[nodiscard]] bool getPreparingCarrier() noexcept {
		auto iter = carriers.end();
		while (iter != carriers.begin())
			if ((--iter)->isReady()) {
				preparing = &*iter;
				return false;
			}
		preparing = nullptr;
		return true;
	}

public:
	DirectTexture(DirectTextureManager* manager) : manager(manager) {}
	void newFile(const unsigned int idPack, String&& file) noexcept { carriers.emplace_back(idPack, std::move(file)); }
	void newFile(unsigned int idPack, String&& file, const DirectTextureLoaderEntry& recommendedLoader) noexcept;
	void removeFile(const unsigned int idPack) noexcept { std::erase_if(carriers, [idPack](DirectTextureCarrier& carrier) -> bool { return carrier.pack == idPack && (carrier.unload(), true); }); }

	/**
	 * @brief 加载尽可能更末端的纹理，直至成功某一个
	 * @return true - 成功，false - 所有的纹理都加载失败
	 */
	bool load() noexcept {
		if (carriers.empty()) return false;
		auto iter = carriers.end();
		while (iter != carriers.begin()) {
			--iter;
			if (iter->isReady() || iter->load(manager)) return preparing = &*iter, true;
		}
		return false;
	}

	void unload() noexcept {
		if (carriers.empty()) return;
		for (DirectTextureCarrier& carrier : carriers) carrier.unload();
	}

	void upload() noexcept {
		if (!preparing && getPreparingCarrier()) return;
		preparing->upload();
	}

	[[nodiscard]] bool isReady() noexcept { return preparing || !getPreparingCarrier(); }

	[[nodiscard]] DirectTextureIndex getIndex() noexcept {
		if (!preparing && getPreparingCarrier()) return -1;
		return preparing->index;
	}
};

class DirectTextureEntry {
	friend class DirectTextureManager;
	DirectTexture* texture;
	DirectTextureEntry(DirectTexture* texture) : texture(texture) {}

public:
	void newFile(const unsigned int idPack, String file) const noexcept { texture->newFile(idPack, std::move(file)); }
	void newFile(unsigned int idPack, String file, const DirectTextureLoaderEntry& recommendedLoader) const noexcept;
	void load() const noexcept { texture->load(); }
	void upload() const noexcept { texture->unload(); }
};

/**
 * 所有的加载器应当继承这个类，并重写保护虚拟函数
 * @code
 * bool onLoad(DirectTextureCarrier& carrier) noexcept override {}
 * void onUnload(DirectTextureCarrier& carrier) noexcept override {}
 * void onUpload(DirectTextureCarrier& carrier) noexcept override {}
 * @endcode
 * 不需要手动管理状态，是否加载成功只需要在 @code DirectTextureLoader::onLoad @endcode 的返回值指定即可。
 */
class DirectTextureLoader {
protected:
	/**
	 * 加载纹理。同时，加载器负责将DirectTextureCarrier的准备类型设置为对应的类型。
	 * @param carrier 加载的纹理资源携带器
	 * @return true - 加载成功，否则加载失败。加载失败时，不调用unload
	 */
	virtual bool onLoad(DirectTextureCarrier& carrier) noexcept = 0;
	virtual void onUnload(DirectTextureCarrier& carrier) noexcept = 0;
	virtual void onUpload(DirectTextureCarrier& carrier) noexcept = 0;
	static void setTextureType(DirectTextureCarrier& carrier, const DirectTextureCarrier::PrepareType type) noexcept { carrier.type = type; }

public:
	virtual ~DirectTextureLoader() = default;

	void load(DirectTextureCarrier& carrier) noexcept { if (carrier.status == DirectTextureCarrier::Register && (carrier.type = DirectTextureCarrier::Unknown, onLoad(carrier))) carrier.status = DirectTextureCarrier::Ready; }
	void unload(DirectTextureCarrier& carrier) noexcept { if (carrier.status == DirectTextureCarrier::Ready) onUnload(carrier), carrier.status = DirectTextureCarrier::Register, carrier.type = DirectTextureCarrier::Unknown; }
	void upload(DirectTextureCarrier& carrier) noexcept { if (carrier.status == DirectTextureCarrier::Ready) onUpload(carrier), carrier.status = DirectTextureCarrier::Uploaded; }
	[[nodiscard]] virtual bool matches(const DirectTextureCarrier& carrier) const noexcept { return false; }
};

class DirectTextureLoaderEntry {
	friend class DirectTextureManager;
	friend class DirectTextureCarrier;
	friend class DirectTexture;
	DirectTextureLoader* loader;
	DirectTextureLoaderEntry(DirectTextureLoader* loader) : loader(loader) {}

public:
	void load(DirectTextureCarrier& carrier) const noexcept { if (loader) loader->load(carrier); }
	void unload(DirectTextureCarrier& carrier) const noexcept { if (loader) loader->unload(carrier); }
	void upload(DirectTextureCarrier& carrier) const noexcept { if (loader) loader->upload(carrier); }
};

class DirectTextureManager {
	Map<String, DirectTexture> textures;
	Map<String, DirectTextureLoader*> loaders;

	void unregisterLoaders() noexcept {
		for (auto& [id, loader] : loaders) delete loader;
		loaders.clear();
	}

public:
	/**
	 * @brief 注册纹理ID
	 * @param id 纹理ID
	 * @return 0 - 成功；otherwise - 失败
	 */
	int registerTexture(const String& id) noexcept {
		if (textures.contains(id)) return -1;
		textures.emplace(id, this);
		return 0;
	}

	/**
	 * @brief 注册加载器
	 * @param id 加载器ID
	 * @return 0 - 成功；otherwise - 失败
	 */
	template <typename Loader> requires requires { new Loader(); } && std::is_base_of_v<DirectTextureLoader, Loader> // 允许模板加载器（继承者）
	int registerLoader(const String& id) noexcept {
		if (loaders.contains(id)) return -1;
		loaders.emplace(id, new Loader());
		return 0;
	}

	/**
	 * @param id 纹理ID
	 * @return 纹理入口点。如果未找到纹理，入口点是无效入口点。
	 */
	DirectTextureEntry getTexture(const String& id) noexcept {
		const auto iter = textures.find(id);
		if (iter == textures.end()) return nullptr;
		return &iter->second;
	}

	[[nodiscard]] DirectTextureLoaderEntry findMatchingLoader(const DirectTextureCarrier& carrier) const noexcept {
		for (const auto& [id, loader] : loaders) if (loader->matches(carrier)) return loader;
		return nullptr;
	}
};

inline bool DirectTextureCarrier::load(DirectTextureManager* manager) noexcept {
	if (!loader) loader = manager->findMatchingLoader(*this).loader;
	if (loader) loader->load(*this);
	return isReady();
}

inline void DirectTextureCarrier::unload() noexcept { if (loader && isReady()) loader->unload(*this); }
inline void DirectTextureCarrier::upload() noexcept { if (loader && isReady()) loader->upload(*this); }

inline void DirectTexture::newFile(const unsigned int idPack, String&& file, const DirectTextureLoaderEntry& recommendedLoader) noexcept { carriers.emplace_back(idPack, std::move(file), recommendedLoader.loader); }
inline void DirectTextureEntry::newFile(const unsigned int idPack, String file, const DirectTextureLoaderEntry& recommendedLoader) const noexcept { texture->newFile(idPack, std::move(file), recommendedLoader); }


struct BMPLoadResult {
	std::vector<uint8_t> pixelData; // RGBA数据
	unsigned int width; // 纹理宽度
	unsigned int height; // 纹理高度
	DXGI_FORMAT format; // 纹理格式
	bool success; // 加载是否成功
};

BMPLoadResult LoadBMPToD3D12Buffer(const std::wstring& filename) {
	BMPLoadResult result = {};
	// 1. 读取文件
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		result.success = false;
		return result;
	}
	// 2. 读取文件头
	BITMAPFILEHEADER fileHeader;
	file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
	// 验证BMP格式
	if (fileHeader.bfType != 0x4D42) { // "BM"
		result.success = false;
		return result;
	}
	// 3. 读取信息头
	BITMAPINFOHEADER infoHeader;
	file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
	// 只支持常见的未压缩格式
	if (infoHeader.biCompression != 0) { // BI_RGB
		result.success = false;
		return result;
	}
	// 只支持24bpp或32bpp
	if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) {
		result.success = false;
		return result;
	}
	result.width = infoHeader.biWidth;
	result.height = abs(infoHeader.biHeight); // 处理倒序存储
	const bool isTopDown = infoHeader.biHeight < 0;
	// 4. 计算像素数据参数
	const unsigned long long bytesPerPixel = infoHeader.biBitCount / 8;
	const unsigned long long rowPitch = result.width * bytesPerPixel + 3 & ~3; // 4字节对齐
	const unsigned long long pixelDataSize = rowPitch * result.height;
	// 5. 定位并读取像素数据
	file.seekg(fileHeader.bfOffBits, std::ios::beg);
	std::vector<uint8_t> bmpPixelData(pixelDataSize);
	file.read(reinterpret_cast<char*>(bmpPixelData.data()), pixelDataSize);
	// 6. 转换为DXGI格式 (BGRA -> RGBA)
	result.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	result.pixelData.resize(result.width * result.height * 4); // RGBA
	for (uint32_t y = 0; y < result.height; ++y) {
		// BMP可能是倒序存储，需要翻转
		uint32_t srcY = isTopDown ? y : (result.height - 1 - y);
		const uint8_t* srcRow = bmpPixelData.data() + srcY * rowPitch;
		uint8_t* dstRow = result.pixelData.data() + y * result.width * 4;

		for (uint32_t x = 0; x < result.width; ++x) {
			const uint8_t* srcPixel = srcRow + x * bytesPerPixel;
			uint8_t* dstPixel = dstRow + x * 4;

			if (bytesPerPixel == 3) { // 24bpp BGR -> RGBA
				dstPixel[0] = srcPixel[2]; // R
				dstPixel[1] = srcPixel[1]; // G
				dstPixel[2] = srcPixel[0]; // B
				dstPixel[3] = 255; // A
			}
			else { // 32bpp BGRA -> RGBA
				dstPixel[0] = srcPixel[2]; // R
				dstPixel[1] = srcPixel[1]; // G
				dstPixel[2] = srcPixel[0]; // B
				dstPixel[3] = srcPixel[3]; // A
			}
		}
	}

	result.success = true;
	return result;
}
