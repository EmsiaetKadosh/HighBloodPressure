//
// Created by EmsiaetKadosh on 25-1-21.
//

#pragma once

#include "..\def.h"

class [[carlbeks::predecl]] ITexture;
class [[carlbeks::predecl]] ITextureManager;

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
	virtual void initialize() noexcept = 0;
	ITextureManager() { initializerChecker.requiredModule(L"TextureManager", L"gc").registerModule(L"TextureManager"); }
	virtual ~ITextureManager(); // unload交给renderer

	[[nodiscard]] TextureEntry getTexture(const String& id) noexcept;
	[[nodiscard]] bool isNullTexture(const TextureEntry& entry) const noexcept { return isNullTexture(entry.texture); }
	[[nodiscard]] virtual TextureEntry getNullTexture() const noexcept = 0;
	[[nodiscard]] virtual TextureEntry loadSingle(const String& id) noexcept = 0;
	virtual void load() noexcept(false) = 0;
	virtual void unload() noexcept(false) = 0;
	void reload() noexcept(false) { unload(), load(); }
};
