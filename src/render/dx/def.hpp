
#pragma once

#include <vector>
#include <sstream>
#include "src\utils\chars.hpp"

struct TextureVertex {
	float position[3]; // 位置 (R32G32B32_FLOAT)
	unsigned int color; // 颜色 (R8G8B8A8_UNORM，需手动归一化)
	float uv[2]; // UV坐标 (R32G32_FLOAT，贴图时使用)
	short normal[3]; // 法线 (R16G16B16_SNORM，光照时使用)
	unsigned short index = 0; // 纹理索引！

	[[nodiscard]] String toString() const noexcept {
		std::wostringstream str;
		str <<
			L"  position: (" << position[0] << L", " << position[1] << L", " << position[2] << L")\n"
			L"  uv      : (" << uv[0] << L", " << uv[1] << L")\n"
			L"  color   : #" << qwtowb16(color) << L"   index: " << index << L"\n"
			L"  normal  : (" << normal[0] / 32767.0 << L", " << normal[1] / 32767.0 << L", " << normal[2] / 32767.0 << L")";
		return str.str();
	}
};

struct ColoredVertex {
	float position[3]; // 位置 (R32G32B32_FLOAT)
	unsigned int color; // 颜色 (R8G8B8A8_UNORM，需手动归一化)
};

template <typename T> requires std::is_same_v<T, TextureVertex> || std::is_same_v<T, ColoredVertex>
class VertexSet {
	friend struct DirectFrame;
	friend class DirectRenderer;

public:
	Vector<T> vertices;
	Vector<unsigned int> indices;
};

using ColoredSet = VertexSet<ColoredVertex>;
using TextureSet = VertexSet<TextureVertex>;

class ColoredSetBuilder {
	ColoredSet set;
	unsigned int count = 0;
	unsigned int shapeBegin = 0;

	ColoredVertex& push() noexcept {
		++count;
		if (count - shapeBegin > 2) { // 满3个以后添加三角形
			set.indices.push_back(shapeBegin);
			set.indices.push_back(count - 2);
			set.indices.push_back(count - 1);
		}
		set.vertices.emplace_back();
		return set.vertices.back();
	}

	// 检查当前是否能够this***，否则先push
	ColoredVertex& ensureThis() noexcept {
		if (count > shapeBegin) return set.vertices.back();
		return push();
	}

public:
	ColoredSetBuilder& predictPolygonEdges(const unsigned int c) noexcept {
		if (c < 3) return *this;
		return predictVertexAmount(c), predictIndexAmount(3 * c - 6);
	}

	ColoredSetBuilder& predictVertexAmount(const unsigned int c) noexcept {
		if (c > 2) set.vertices.reserve(c);
		return *this;
	}

	ColoredSetBuilder& predictIndexAmount(const unsigned int c) noexcept {
		if (c > 3) set.indices.reserve(c);
		return *this;
	}

	ColoredSetBuilder& nextColored(const unsigned int color) noexcept {
		push().color = color;
		return *this;
	}

	ColoredSetBuilder& thisColored(const unsigned int color) noexcept {
		ensureThis().color = color;
		return *this;
	}

	ColoredSetBuilder& nextAt(const Vector3D& v) noexcept {
		auto& [position, color] = push();
		position[0] = static_cast<float>(v.getX());
		position[1] = static_cast<float>(v.getY());
		position[2] = static_cast<float>(v.getZ());
		return *this;
	}

	ColoredSetBuilder& thisAt(const Vector3D& v) noexcept {
		auto& [position, color] = ensureThis();
		position[0] = static_cast<float>(v.getX());
		position[1] = static_cast<float>(v.getY());
		position[2] = static_cast<float>(v.getZ());
		return *this;
	}

	ColoredSetBuilder& nextAt(const float x, const float y, const float z) noexcept {
		auto& [position, color] = push();
		position[0] = x;
		position[1] = y;
		position[2] = z;
		return *this;
	}

	ColoredSetBuilder& thisAt(const float x, const float y, const float z) noexcept {
		auto& [position, color] = ensureThis();
		position[0] = x;
		position[1] = y;
		position[2] = z;
		return *this;
	}

	ColoredSetBuilder& nextAt(const double x, const double y, const double z) noexcept {
		auto& [position, color] = push();
		position[0] = static_cast<float>(x);
		position[1] = static_cast<float>(y);
		position[2] = static_cast<float>(z);
		return *this;
	}

	ColoredSetBuilder& thisAt(const double x, const double y, const double z) noexcept {
		auto& [position, color] = ensureThis();
		position[0] = static_cast<float>(x);
		position[1] = static_cast<float>(y);
		position[2] = static_cast<float>(z);
		return *this;
	}

	ColoredSetBuilder& nextPolygon() noexcept {
		shapeBegin = count;
		return *this;
	}

	ColoredSetBuilder& print(std::wostringstream& str) noexcept {
		str << L"ColoredSetBuilder:";
		for (size_t i = 0; i < set.vertices.size(); ++i) {
			auto& [position, color] = set.vertices[i];
			str << L"\n   " << qwtowb10(i, 4) << L' ' << position[0] << L", " << position[1] << L", " << position[2] << L' ' << qwtowb16(color, 8);
		}
		return *this;
	}

	ColoredSet build() noexcept {
		ColoredSet ret = std::move(set);
		set = ColoredSet();
		return ret;
	}
};

class DirectTextureIndex {
	friend class DirectTexture;
	friend class DirectTextureContext;
	friend struct DirectFrame;
	friend struct DirectTextureResource;

	enum BufferType : unsigned char {
		Invalid = 0,
		ConstantBuffer = 1,
		PreloadBuffer = 2,
		AppendBuffer = 3,
		TemporaryBuffer = 4
	} bufferType = Invalid;

	unsigned short bufferIndex = 0;
	unsigned char spriteIndex = 0; // If single texture or non-sprite index, this is ignored

	DirectTextureIndex(decltype(-1)) : DirectTextureIndex() {}
	DirectTextureIndex(const BufferType type, const unsigned short index) : bufferType(type), bufferIndex(index) {}

public:
	DirectTextureIndex() = default;
	[[nodiscard]] operator bool() const noexcept { return bufferType != Invalid; }
	[[nodiscard]] bool operator!() const noexcept { return bufferType == Invalid; }
};


class DirectTextureContext {
	DirectTextureIndex index;

public:
	Vector<TextureVertex> vertices; // 可以自定义拉伸图形。公开成员，允许一些自由修改

	DirectTextureContext& of(const DirectTextureIndex& index) noexcept { return this->index = index, *this; }
	DirectTextureContext& atLeftTop(const Vector3D& v, const unsigned int color) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { 0.0f, 0.0f }, .normal = { 0, 0, 0 } }), *this; }
	DirectTextureContext& atLeftBottom(const Vector3D& v, const unsigned int color) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { 0.0f, 1.0f }, .normal = { 0, 0, 0 } }), *this; }
	DirectTextureContext& atRightTop(const Vector3D& v, const unsigned int color) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { 1.0f, 0.0f }, .normal = { 0, 0, 0 } }), *this; }
	DirectTextureContext& atRightBottom(const Vector3D& v, const unsigned int color) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { 1.0f, 1.0f }, .normal = { 0, 0, 0 } }), *this; }
	DirectTextureContext& at(const Vector3D& v, const unsigned int color, const Vector2D& uv) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { static_cast<float>(uv.getX()), static_cast<float>(uv.getY()) }, .normal = { 0, 0, 0 } }), *this; }
};
