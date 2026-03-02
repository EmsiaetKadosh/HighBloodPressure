
#pragma once

#include <vector>
#include <sstream>
#include "src\utils\chars.hpp"

enum class DirectResourceBufferType {
	Null,
	Temporary,
	Append,
	Preload,
	Constant
};

struct TextureVertex {
	float position[3]; // 位置 (R32G32B32_FLOAT)
	unsigned int color; // 颜色 (R8G8B8A8_UNORM，需手动归一化)
	float uv[2]; // UV坐标 (R32G32_FLOAT，贴图时使用)
	short normal[3]; // 法线 (R16G16B16_SNORM，光照时使用)
	unsigned short index = 0; // 纹理索引！

	[[nodiscard]] String toString() const {
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

	ColoredSetBuilder& print(std::wostringstream& str) {
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

/**
 * @brief 纹理索引。指示纹理被存放在上传缓冲区的uv偏移、索引。
 * 此类无需外部使用
 */
class DirectTextureIndex {
	friend class DirectTextureDispatcher;

	unsigned int index = 0; // 内置拼合纹理索引
	unsigned int xGrid = 0, yGrid = 0; // 网格起始索引
	unsigned int xGridSize = 0, yGridSize = 0; // 网格尺寸索引
	float xSample = 0.0f, ySample = 0.0f; // 采样起始uv
	float xSampleSize = 0.0f, ySampleSize = 0.0f; // 采样尺寸uv
	DirectResourceBufferType buffer = DirectResourceBufferType::Null;
	[[nodiscard]] bool isNull() const noexcept { return buffer == DirectResourceBufferType::Null; }
	[[nodiscard]] operator bool() const noexcept { return !isNull(); }
	[[nodiscard]] bool operator!() const noexcept { return isNull(); }

public:
	static const DirectTextureIndex& ofNull() noexcept {
		static DirectTextureIndex null;
		return null;
	}
};

class DirectTextureEntry {
	friend class DirectTextureManager;
	friend class DirectTextureContext;
	class DirectTextureCarrier* carrier = nullptr;

	DirectTextureCarrier* replace(DirectTextureCarrier* newCarrier) noexcept {
		DirectTextureCarrier* const ret = carrier;
		carrier = newCarrier;
		return ret;
	}
};

class DirectTextureContext {
	DirectTextureCarrier* carrier = nullptr;

public:
	Vector<TextureVertex> vertices; // 可以自定义拉伸图形。公开成员，允许一些自由修改

	DirectTextureContext& of(const DirectTextureEntry& entry) noexcept { return this->carrier = entry.carrier, *this; }
	DirectTextureContext& atLeftTop(const Vector3D& v, const unsigned int color) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { 0.0f, 0.0f }, .normal = { 0, 0, 0 } }), *this; }
	DirectTextureContext& atLeftBottom(const Vector3D& v, const unsigned int color) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { 0.0f, 1.0f }, .normal = { 0, 0, 0 } }), *this; }
	DirectTextureContext& atRightTop(const Vector3D& v, const unsigned int color) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { 1.0f, 0.0f }, .normal = { 0, 0, 0 } }), *this; }
	DirectTextureContext& atRightBottom(const Vector3D& v, const unsigned int color) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { 1.0f, 1.0f }, .normal = { 0, 0, 0 } }), *this; }
	DirectTextureContext& at(const Vector3D& v, const unsigned int color, const Vector2D& uv) noexcept { return vertices.emplace_back(TextureVertex { .position = { static_cast<float>(v.getX()), static_cast<float>(v.getY()), static_cast<float>(v.getZ()) }, .color = color, .uv = { static_cast<float>(uv.getX()), static_cast<float>(uv.getY()) }, .normal = { 0, 0, 0 } }), *this; }
};
