//
// Created by EmsiaetKadosh on 25-1-21.
//

#include "..\utils\utils.h"
#include "..\utils\exception.h"
#include "TextureManager.h"

TextureEntry ITextureManager::getTexture(const String& id) noexcept {
	if (const auto iter = textures.find(id); iter != textures.cend()) return TextureEntry(iter->second.ptr());
	Logger.error(L"Texture not loaded: " + id + L". Trying to load...");
	printStacktrace();
	return loadSingle(id);
}
