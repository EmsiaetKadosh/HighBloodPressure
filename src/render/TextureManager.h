//
// Created by EmsiaetKadosh on 25-1-21.
//

#pragma once

#include "..\def.h"

class [[carlbeks::predecl]] ITexture;
class [[carlbeks::predecl]] GdiTexture;
class [[carlbeks::predecl]] TextureManager;

class ITexture {
protected:
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
	virtual bool load(const String& path) noexcept = 0;
	virtual void release() noexcept = 0;
	virtual void draw(unsigned int left, unsigned int top, double scale) noexcept = 0;
};

class GdiTexture final : public ITexture {
public:
	GdiTexture(const String& path) : ITexture(path) {}
	GdiTexture(String&& path) : ITexture(std::move(path)) {}
	~GdiTexture() override { release(); }
	bool load(const String& path) noexcept override {

	}
	void release() noexcept override;
	void draw(unsigned left, unsigned top, double scale) noexcept override;
};

class TextureManager {
	Map<String, Container<ITexture>> textures;
	using IterTexture = Map<String, Container<ITexture>>::const_iterator;

public:
	TextureManager() = default;
};

inline static TextureManager textureManager = TextureManager();
