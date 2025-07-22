//
// Created by EmsiaetKadosh on 25-6-18.
//

#pragma once

#include "..\TextureManager.h"

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

class GdiTextureManager final : public ITextureManager {
	mutable GdiTexture null;

	[[nodiscard]] bool isNullTexture(ITexture* texture) const noexcept override { return texture == &null; }

public:
	void initialize() noexcept override;
	GdiTextureManager();
	~GdiTextureManager();
	[[nodiscard]] TextureEntry getNullTexture() const noexcept override { return entryOf(&null); }
	void load() noexcept(false) override;
	void unload() noexcept(false) override;
	TextureEntry loadSingle(const String& id) noexcept override;
};
