//
// Created by EmsiaetKadosh on 25-1-21.
//

#pragma once

#include "..\def.h"

class [[carlbeks::predecl]] ITexture;
class [[carlbeks::predecl]] GdiTexture;
class [[carlbeks::predecl]] ITextureManager;
class [[carlbeks::predecl]] GdiTextureManager;

class ITexture {
protected:
	friend class [[carlbeks::predecl, carlbeks::defineat("renderer.h")]] IRenderer;
	friend class [[carlbeks::predecl]] ITextureManager;
	unsigned int width = 0, height = 0;
	String path;
	ITexture(String&& path) : path(std::move(path)) {}
	ITexture(const String& path) : path(path) {}

public:
	ITexture(const ITexture&) = delete;
	ITexture(ITexture&&) = delete;
	ITexture& operator=(const ITexture&) = delete;
	ITexture& operator=(ITexture&&) = delete;
	virtual ~ITexture() = default;
	virtual int load() noexcept = 0;
	virtual void unload() noexcept = 0;
	[[nodiscard]] const String& getPath() const noexcept { return path; }
	[[nodiscard]] unsigned int getWidth() const noexcept { return width; }
	[[nodiscard]] unsigned int getHeight() const noexcept { return height; }
};

class GdiTexture final : public ITexture {
	friend class GdiTextureManager;
	friend class [[carlbeks::predecl, carlbeks::defineat("renderer.h")]] GdiRenderer;
	HBITMAP bitmap = nullptr;
	bool withSrcAlpha = false;

public:
	GdiTexture(const String& path) : ITexture(path) {}
	GdiTexture(String&& path) : ITexture(std::move(path)) {}
	~GdiTexture() override;
	int load() noexcept override;
	void unload() noexcept override;
};

class TextureEntry {
	friend class [[carlbeks::predecl, carlbeks::defineat("renderer.h")]] IRenderer;
	friend class [[carlbeks::predecl]] ITextureManager;
	ITexture* texture = nullptr;
	TextureEntry(ITexture* texture) : texture(texture) {}

public:
	TextureEntry(const TextureEntry&) = default;
	TextureEntry(TextureEntry&&) = default;
	TextureEntry& operator=(const TextureEntry&) = default;
	TextureEntry& operator=(TextureEntry&&) = default;
	~TextureEntry() = default;
	[[nodiscard]] bool isNullTexture() const noexcept;
};

class ITextureManager {
protected:
	Map<String, Container<ITexture>> textures;
	using IterTexture = Map<String, Container<ITexture>>::const_iterator;

	static TextureEntry entryOf(ITexture* texture) noexcept { return TextureEntry(texture); }
	[[nodiscard]] virtual bool isNullTexture(ITexture* texture) const noexcept = 0;

public:
	ITextureManager() = default;
	virtual ~ITextureManager() = default;

	[[nodiscard]] TextureEntry getTexture(const String& id) const noexcept {
		if (const auto iter = textures.find(id); iter != textures.cend()) return TextureEntry(iter->second.ptr());
		return getNullTexture();
	}

	[[nodiscard]] bool isNullTexture(const TextureEntry& entry) const noexcept { return isNullTexture(entry.texture); }
	void reload() noexcept(false) { unload(), load(); }
	[[nodiscard]] virtual TextureEntry getNullTexture() const noexcept = 0;
	virtual void load() noexcept(false) = 0;
	virtual void unload() noexcept(false) = 0;
};

class GdiTextureManager final : public ITextureManager {
	mutable GdiTexture null;

	[[nodiscard]] bool isNullTexture(ITexture* texture) const noexcept override { return texture == &null; }

public:
	GdiTextureManager();
	[[nodiscard]] TextureEntry getNullTexture() const noexcept override { return entryOf(&null); }
	void load() noexcept(false) override;
	void unload() noexcept(false) override;
};

extern ITextureManager& textureManager;
