//
// Created by EmsiaetKadosh on 25-6-18.
//

#pragma message("using GDI file " __FILE__)

#include "texture.hpp"
#include "renderer.hpp"
#include "..\..\game\Game.h"

GdiTexture::~GdiTexture() { if (bitmap) Logger.of(L"Texture not unloaded:", path).error(); }

int GdiTexture::load() noexcept {
	const String realPath = LR"(.\assets\texture\)" + path + L".bmp";
	Logger.of(L"Loading texture:", path, L"from", realPath).info();
	bitmap = static_cast<HBITMAP>(LoadImageW(nullptr, realPath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION));
	if (!bitmap) {
		Logger.of(L"Failed to load texture.", path, L"LastError:", GetLastError()).error();
		Failed();
	}
	BITMAP map;
	GetObjectW(bitmap, sizeof(map), &map);
	width = map.bmWidth;
	height = map.bmHeight;
	Success();
}

void GdiTexture::unload() noexcept { if (bitmap) static_cast<GdiRenderer&>(renderer).deleteObject(bitmap), bitmap = nullptr; }
bool TextureEntry::isNullTexture() const noexcept { return textureManager.isNullTexture(*this); }

void GdiTextureManager::initialize() noexcept {
	if (!null.bitmap) { // 空纹理修复
		const GdiRenderer& gdi = static_cast<GdiRenderer&>(renderer);
		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = 1;
		bmi.bmiHeader.biHeight = -1; // 负值表示从上到下的位图
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		void* pBits = nullptr;
		null.bitmap = CreateDIBSection(gdi.MainDC, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
		if (null.bitmap && pBits) {
			*static_cast<DWORD*>(pBits) = 0xFF000000;
			null.width = 1;
			null.height = 1;
		} else Logger.error(L"Failed to create 1x1 bitmap for null");
	}
}

GdiTextureManager::GdiTextureManager(): null(L"null") {}
GdiTextureManager::~GdiTextureManager() { unload(); }

void GdiTextureManager::load() noexcept(false) {
	if (game.options.resource.lazyLoad) return;
	null.load();
	for (auto& [id, texture] : textures)
		if (texture->load()) {
			Logger.error(L"Failed to load texture: " + texture->getPath());
			throw std::runtime_error("Failed to load texture");
		}
}

void GdiTextureManager::unload() noexcept(false) {
	null.unload();
	for (auto& [id, texture] : textures) texture->unload();
}

TextureEntry GdiTextureManager::loadSingle(const String& id) noexcept {
	if (const auto iter = textures.find(id); iter != textures.cend()) {
		iter->second->unload();
		if (iter->second->load()) Logger.error(L"Failed to load texture: " + id);
		return entryOf(iter->second);
	}
	GdiTexture* texture;
	textures.emplace(id, Container<ITexture>(&texture, id));
	texture->load();
	return entryOf(texture);
}

namespace $LimitedAccess {
	ITextureManager& getTextureManager() noexcept {
		static GdiTextureManager textureManager;
		return textureManager;
	}
}
