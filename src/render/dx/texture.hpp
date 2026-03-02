
#pragma once

#include <map>
#include <unordered_set>
#include <utility>

#include "src\using.hpp"

class DirectTextureCarrier;
class DirectTextureEntry;
class DirectTextureLoader;
class DirectTextureLoaderEntry;
class DirectTextureManager;

/**
 * 纹理加载后的实际数据应该就存放在这里
 */
struct DirectTextureCarrierExtraData {
	virtual ~DirectTextureCarrierExtraData() noexcept = default;
	virtual unsigned int getBufferSize() noexcept = 0;
	virtual unsigned char* getBuffer() noexcept = 0;
};

/**
 * 每个纹理文件占据一个DirectTextureCarrier。
 * 将DirectTextureCarrier上传到DirectTexture中形成纹理集合（也就是内部精灵图）；
 * 或者DirectTextureCarrier本身设置为精灵图，则其自身将占据一个纹理集合。
 */
class DirectTextureCarrier {
	friend class DirectTextureLoader;
	friend class DirectTextureDispatcher;
	friend struct DirectTextureResource;
	String file; // align 8
	DirectTextureLoader* loader = nullptr; // 8
	DirectTextureCarrierExtraData* extraData = nullptr;
	unsigned int width = 0, height = 0; // 4 + 4
	DirectTextureIndex index = DirectTextureIndex::ofNull(); // 4

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

	/**
	 * @brief 加载纹理到目标管理器
	 * @param manager 纹理管理器
	 * @return true - 准备完毕，可以使用
	 */
	[[nodiscard]] bool load(DirectTextureManager* manager) noexcept;
	void unload() noexcept;
	void upload() noexcept;
	[[nodiscard]] const String& getFile() const noexcept { return file; }
	[[nodiscard]] bool isReady() const noexcept { return status == Ready || status == Uploaded; }

public:
	DirectTextureCarrier(String&& file) : file(std::move(file)) {}
	DirectTextureCarrier(String&& file, DirectTextureLoader* loader) : file(std::move(file)), loader(loader) {}
	~DirectTextureCarrier() noexcept = default;
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
	DirectTextureLoader* loader;
	DirectTextureLoaderEntry(DirectTextureLoader* loader) : loader(loader) {}

public:
	void load(DirectTextureCarrier& carrier) const noexcept { if (loader) loader->load(carrier); }
	void unload(DirectTextureCarrier& carrier) const noexcept { if (loader) loader->unload(carrier); }
	void upload(DirectTextureCarrier& carrier) const noexcept { if (loader) loader->upload(carrier); }
};

/**
 * @brief 管理所有纹理资源的内存
 */
class DirectTextureManager {
	friend class DirectRenderer;
	Map<String, DirectTextureEntry> textures;
	Map<String, DirectTextureLoader*> loaders;
	DirectRenderer& renderer;
	const DirectTextureEntry nullEntry;

	void unregisterLoaders() noexcept {
		for (auto& [id, loader] : loaders) delete loader;
		loaders.clear();
	}

	DirectTextureManager(DirectRenderer& renderer) noexcept : renderer(renderer) {}

public:
	/**
	 * @brief 注册纹理ID
	 * @param id 纹理ID
	 * @param file 纹理文件
	 * @param loader 纹理加载器
	 * @return 0 - 成功；otherwise - 失败
	 */
	int registerTexture(const String& id, String&& file, const DirectTextureLoaderEntry loader) noexcept {
		const auto& [iter, flag] = textures.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
		if (!flag) return -1; // 未发生插入
		iter->second.carrier = new DirectTextureCarrier(std::move(file), loader.loader);
		return 0;
	}

	/**
	 * @brief 强行更新一个纹理
	 * @param id 纹理ID
	 * @param file 纹理文件
	 * @param loader 纹理加载器
	 */
	void updateTexture(const String& id, String&& file, const DirectTextureLoaderEntry loader) noexcept {
		const auto& [iter, flag] = textures.try_emplace(id);
		const DirectTextureCarrier* old = iter->second.replace(new DirectTextureCarrier(std::move(file), loader.loader));
		delete old;
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
	 * @brief 根据ID获取纹理
	 * @param id 纹理ID
	 * @return 纹理入口点。如果未找到纹理，入口点是无效入口点。
	 */
	[[deprecated]] const DirectTextureEntry& getTexture(const String& id) noexcept {
		const auto iter = textures.find(id);
		if (iter == textures.end()) return nullEntry;
		return iter->second;
	}

	[[nodiscard]] DirectTextureLoaderEntry findMatchingLoader(const DirectTextureCarrier& carrier) const noexcept {
		for (const auto& [id, loader] : loaders) if (loader->matches(carrier)) return loader;
		return nullptr;
	}
};

/**
 * @brief 准备一个场景的PB AB TB的内容；
 * 准备完成后，尝试添加进入PB的内容会被自动添加进AB
 */
class DirectTextureScene {
	friend class DirectTextureResourceManager;
	DirectTextureManager* manager;
	HashSet<DirectTextureCarrier*> preload;
	HashSet<DirectTextureCarrier*> append;
	HashSet<DirectTextureCarrier*> temporary;
	/**
	 * @brief 此变量用于提高效率。
	 * 每次Scene中的内容发生更改时，此值+1。
	 * 渲染提交时，查看DTR中的版本值与此值是否相同，不同则重新打包上传。
	 * 主要是append部分。
	 * 参考@code DirectTextureResource::version @endcode。
	 */
	unsigned int version = 0;
	bool ok = false;

	bool checkExistence(DirectTextureCarrier* const carrier) const noexcept { return carrier && (preload.contains(carrier) || append.contains(carrier) || temporary.contains(carrier)); }

public:
	DirectTextureScene(DirectTextureManager* manager) noexcept : manager(manager) {}

	DirectTextureScene& addPreload(DirectTextureCarrier* const carrier) noexcept { return checkExistence(carrier) || ((ok ? append : preload).emplace(carrier), true), *this; }
	DirectTextureScene& addAppend(DirectTextureCarrier* const carrier) noexcept { return checkExistence(carrier) || (append.emplace(carrier), ++version, true), *this; }
	DirectTextureScene& addTemporary(DirectTextureCarrier* const carrier) noexcept { return checkExistence(carrier) || (temporary.emplace(carrier), true), *this; }
};

inline bool DirectTextureCarrier::load(DirectTextureManager* manager) noexcept {
	if (!loader) loader = manager->findMatchingLoader(*this).loader;
	if (loader) loader->load(*this);
	return isReady();
}

inline void DirectTextureCarrier::unload() noexcept { if (loader && isReady()) loader->unload(*this); }
inline void DirectTextureCarrier::upload() noexcept { if (loader && isReady()) loader->upload(*this); }
