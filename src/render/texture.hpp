
#pragma once

#include <string>
#include <unordered_set>
#include "src\using.hpp"

class Texture {
	friend class TextureManager;
	String name;
	class DirectTextureCarrier* carrier;
public:
	const String& getName() noexcept { return name; }

	struct Hash : std::hash<String> {
		[[nodiscard]] size_t operator()(const Texture& t) const noexcept(noexcept(std::hash<String>()(t.name))) { return std::hash<String>::operator()(t.name); }
	};
};

class TextureManager {
	HashSet<Texture, Texture::Hash> textures;
public:
	void initialize() noexcept;
	void finalize() noexcept;
};
