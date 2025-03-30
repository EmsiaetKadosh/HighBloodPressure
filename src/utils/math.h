//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\warnings.h"
#include "..\def.h"

template <typename T>
const T& nMin(const T& a, const T& b) { return a < b ? a : b; }

template <typename T>
const T& nMin(const T& val0, const T& val1, const T& vals...) { return nMin(nMin(val0, val1), nMin(vals...)); }

template <typename T>
const T& nMax(const T& a, const T& b) { return a > b ? a : b; }

template <typename T>
const T& nMax(const T& val0, const T& val1, const T& vals...) { return nMax(nMin(val0, val1), nMax(vals...)); }

template <typename T>
const T& nRange(const T& val, const T& min, const T& max) { return nMax(nMin(val, max), min); }

template <typename T>
bool nBetween(const T& val, const T& min, const T& max) { return val >= min && val <= max; }

template <typename T>
bool nSamePositivity(const T& val, const T& other) { return (val <= 0 && other <= 0) || (val >= 0 && other >= 0); }

template <typename T>
consteval T nConsteval(T val) { return val; }

inline bool dEquals(const double v1, const double v2) { return std::abs(v1 - v2) < DBL_EPSILON; }

template <typename T>
void nMinMaxOf(QWORD& outMin, QWORD& outMax, const QWORD total, const T values[]) {
	outMin = outMax = 0;
	for (QWORD i = 0; i < total; ++i) {
		if (values[i] < values[outMin]) outMin = i;
		if (values[i] > values[outMax]) outMax = i;
	}
}

template <typename T>
T nSquare(const T& val) { return val * val; }


class [[carlbeks::TriviallyCopyable]] Vector3D;
class [[carlbeks::TriviallyCopyable]] Vector2D;

class [[carlbeks::TriviallyCopyable]] Vector3D {
	double x, y, z;

public:
	Vector3D(const double x, const double y, const double z) noexcept : x(x), y(y), z(z) {}
	Vector3D() noexcept : x(0), y(0), z(0) {}
	Vector3D(const Vector3D& other) noexcept = default;
	Vector3D(Vector3D&& other) noexcept = default;
	Vector3D& operator=(const Vector3D& other) noexcept = default;
	Vector3D& operator=(Vector3D&& other) noexcept = default;
	~Vector3D() noexcept = default;
	[[nodiscard]] double getX() const noexcept { return x; }
	[[nodiscard]] double getY() const noexcept { return y; }
	[[nodiscard]] double getZ() const noexcept { return z; }
	[[nodiscard]] Vector3D clone() const noexcept { return Vector3D(x, y, z); }
	[[nodiscard]] Vector3D operator+(const Vector3D& other) const noexcept { return Vector3D(x + other.x, y + other.y, z + other.z); }
	[[nodiscard]] Vector3D operator-(const Vector3D& other) const noexcept { return Vector3D(x - other.x, y - other.y, z - other.z); }
	[[nodiscard]] Vector3D operator*(const double scalar) const noexcept { return Vector3D(x * scalar, y * scalar, z * scalar); }
	[[nodiscard]] Vector3D operator/(const double scalar) const noexcept { return Vector3D(x / scalar, y / scalar, z / scalar); }
	[[nodiscard]] Vector3D operator-() const noexcept { return Vector3D(-x, -y, -z); }
	[[nodiscard]] double operator*(const Vector3D& other) const noexcept { return x * other.x + y * other.y + z * other.z; }
	[[nodiscard]] double length() const noexcept { return std::sqrt(x * x + y * y + z * z); }
	[[nodiscard]] double lengthManhattan() const noexcept { return std::abs(x) + std::abs(y) + std::abs(z); }
	bool operator==(const Vector3D& other) const noexcept { return x == other.x && y == other.y && z == other.z; }
	bool operator!=(const Vector3D& other) const noexcept { return x != other.x || y != other.y || z != other.z; }
	Vector3D& operator+=(const Vector3D& other) noexcept { return x += other.x, y += other.y, z += other.z, *this; }
	Vector3D& operator-=(const Vector3D& other) noexcept { return x -= other.x, y -= other.y, z -= other.z, *this; }
	Vector3D& operator*=(const double scalar) noexcept { return x *= scalar, y *= scalar, z *= scalar, *this; }
	Vector3D& operator/=(const double scalar) noexcept { return x /= scalar, y /= scalar, z /= scalar, *this; }
	Vector3D& add(const Vector3D& other) noexcept { return x += other.x, y += other.y, z += other.z, *this; }
	Vector3D& add(const double x, const double y, const double z) noexcept { return this->x += x, this->y += y, this->z += z, *this; }
	Vector3D& subtract(const Vector3D& other) noexcept { return x -= other.x, y -= other.y, z -= other.z, *this; }
	Vector3D& subtract(const double x, const double y, const double z) noexcept { return this->x -= x, this->y -= y, this->z -= z, *this; }
	Vector3D& multiply(const double value) noexcept { return x *= value, y *= value, z *= value, *this; }
	Vector3D& divide(const double value) noexcept { return x /= value, y /= value, z /= value, *this; }
	Vector3D& negate() noexcept { return x = -x, y = -y, z = -z, *this; }
	Vector3D& setX(const double val) noexcept { return x = val, *this; }
	Vector3D& setY(const double val) noexcept { return y = val, *this; }
	Vector3D& setZ(const double val) noexcept { return z = val, *this; }
	Vector3D& flipValueX() noexcept { return x = -x, *this; }
	Vector3D& flipValueY() noexcept { return y = -y, *this; }
	Vector3D& flipValueZ() noexcept { return z = -z, *this; }
	Vector3D& flipAroundX() noexcept { return y = -y, z = -z, *this; }
	Vector3D& flipAroundY() noexcept { return x = -x, z = -z, *this; }
	Vector3D& flipAroundZ() noexcept { return x = -x, y = -y, *this; }
	[[nodiscard]] double dot(const Vector3D& other) const noexcept { return x * other.x + y * other.y + z * other.z; }
	[[nodiscard]] Vector3D cross(const Vector3D& other) const noexcept { return Vector3D(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }
	[[nodiscard]] bool isZero() const noexcept { return x == 0 && y == 0 && z == 0; }
	[[nodiscard]] bool sameDirectionAs(const Vector3D& other) const noexcept { return isZero() || other.isZero() || dot(other) < other.length() * length() * DBL_EPSILON; }

	[[nodiscard]] Vector3D getNormalized() const noexcept {
		if (isZero()) return Vector3D(0.0, 0.0, 0.0);
		const double length = this->length();
		return Vector3D(x / length, y / length, z / length);
	}

	Vector3D& normalize() noexcept {
		if (isZero()) return x = 0, y = 0, z = 0, *this;
		const double length = this->length();
		x /= length, y /= length, z /= length;
		return *this;
	}

	Vector3D& extendValueX(const double target) noexcept {
		if (x == 0) return x = 0, y = 0, z = 0, *this;
		const double c = target / x;
		return x = target, y *= c, z *= c, *this;
	}

	Vector3D& extendValueY(const double target) noexcept {
		if (y == 0) return x = 0, y = 0, z = 0, *this;
		const double c = target / y;
		return x *= c, y = target, z *= c, *this;
	}

	Vector3D& extendValueZ(const double target) noexcept {
		if (z == 0) return x = 0, y = 0, z = 0, *this;
		const double c = target / z;
		return x *= c, y *= c, z = target, *this;
	}

	[[nodiscard]] Vector3D getNearestPointFrom(const Vector3D& point, const Vector3D& direction) const noexcept {
		const Vector3D normalized = direction.getNormalized();
		return point + normalized * clone().subtract(point).dot(normalized);
	}

	[[nodiscard]] String toString() const noexcept { return L"(" + std::to_wstring(x) + L", " + std::to_wstring(y) + L", " + std::to_wstring(z) + L")"; }
};

class [[carlbeks::TriviallyCopyable]] Vector2D {
	double x, y;

public:
	Vector2D(const double x, const double y) noexcept : x(x), y(y) {}
	Vector2D() noexcept : x(0), y(0) {}
	Vector2D(const Vector2D& other) noexcept = default;
	Vector2D(Vector2D&& other) noexcept = default;
	Vector2D& operator=(const Vector2D& other) noexcept = default;
	Vector2D& operator=(Vector2D&& other) noexcept = default;
	~Vector2D() noexcept = default;
	[[nodiscard]] double getX() const noexcept { return x; }
	[[nodiscard]] double getY() const noexcept { return y; }
	[[nodiscard]] Vector2D clone() const noexcept { return Vector2D(x, y); }
	[[nodiscard]] Vector2D operator+(const Vector2D& other) const noexcept { return Vector2D(x + other.x, y + other.y); }
	[[nodiscard]] Vector2D operator-(const Vector2D& other) const noexcept { return Vector2D(x - other.x, y - other.y); }
	[[nodiscard]] Vector2D operator*(const double scalar) const noexcept { return Vector2D(x * scalar, y * scalar); }
	[[nodiscard]] Vector2D operator/(const double scalar) const noexcept { return Vector2D(x / scalar, y / scalar); }
	[[nodiscard]] Vector2D operator-() const noexcept { return Vector2D(-x, -y); }
	[[nodiscard]] double operator*(const Vector2D& other) const noexcept { return x * other.x + y * other.y; }
	[[nodiscard]] double length() const noexcept { return std::sqrt(x * x + y * y); }
	[[nodiscard]] double lengthManhattan() const noexcept { return std::abs(x) + std::abs(y); }
	bool operator==(const Vector2D& other) const noexcept { return x == other.x && y == other.y; }
	bool operator!=(const Vector2D& other) const noexcept { return x != other.x || y != other.y; }
	Vector2D& operator+=(const Vector2D& other) noexcept { return x += other.x, y += other.y, *this; }
	Vector2D& operator-=(const Vector2D& other) noexcept { return x -= other.x, y -= other.y, *this; }
	Vector2D& operator*=(const double scalar) noexcept { return x *= scalar, y *= scalar, *this; }
	Vector2D& operator/=(const double scalar) noexcept { return x /= scalar, y /= scalar, *this; }
	Vector2D& add(const Vector2D& other) noexcept { return x += other.x, y += other.y, *this; }
	Vector2D& add(const double x, const double y) noexcept { return this->x += x, this->y += y, *this; }
	Vector2D& subtract(const Vector2D& other) noexcept { return x -= other.x, y -= other.y, *this; }
	Vector2D& subtract(const double x, const double y) noexcept { return this->x -= x, this->y -= y, *this; }
	Vector2D& multiply(const double value) noexcept { return x *= value, y *= value, *this; }
	Vector2D& divide(const double value) noexcept { return x /= value, y /= value, *this; }
	Vector2D& negate() noexcept { return x = -x, y = -y, *this; }
	Vector2D& setX(const double val) noexcept { return x = val, *this; }
	Vector2D& setY(const double val) noexcept { return y = val, *this; }
	Vector2D& flipValueX() noexcept { return x = -x, *this; }
	Vector2D& flipValueY() noexcept { return y = -y, *this; }
	Vector2D& flipAroundX() noexcept { return y = -y, *this; }
	Vector2D& flipAroundY() noexcept { return x = -x, *this; }
	Vector2D& extendValueX(const double target) noexcept { return x == 0 ? (x = 0, y = 0) : (y *= target / x, x = target), *this; }
	Vector2D& extendValueY(const double target) noexcept { return y == 0 ? (x = 0, y = 0) : (x *= target / y, y = target), *this; }
	[[nodiscard]] double dot(const Vector2D& other) const noexcept { return x * other.x + y * other.y; }
	[[nodiscard]] Vector3D cross(const Vector2D& other) const noexcept { return Vector3D(0, 0, x * other.y - y * other.x); }
	[[nodiscard]] bool isZero() const noexcept { return x == 0.0 && y == 0.0; }
	[[nodiscard]] bool sameDirectionAs(const Vector2D& other) const noexcept { return isZero() || other.isZero() || dot(other) < other.length() * length() * DBL_EPSILON; }

	[[nodiscard]] Vector2D getNormalized() const noexcept {
		if (isZero()) return Vector2D(0.0, 0.0);
		const double length = this->length();
		return Vector2D(x / length, y / length);
	}

	Vector2D& normalize() noexcept {
		if (isZero()) return x = 0, y = 0, *this;
		const double length = this->length();
		x /= length, y /= length;
		return *this;
	}

	/**
	 * 当前机制：
	 * 所有向量可以找到一个最接近的正四向向量，斜四向向量除外。对斜四向向量，返回其顺时针所得第一个正四向向量。
	 * @returns Vector2D 单位向量
	 */
	[[nodiscard]] Vector2D getFourWay() const noexcept {
		if (x > 0) {
			if (y > x) return Vector2D(0, 1);
			if (y <= -x) return Vector2D(0, -1);
			return Vector2D(1, 0);
		}
		if (y < x) return Vector2D(0, -1);
		if (y >= -x) return Vector2D(0, 1);
		return Vector2D(-1, 0);
	}

	/**
	 * 当前机制：所有向量可以找到一个最接近的斜四向向量，正四向向量除外。对正四向向量，返回其顺时针所得第一个斜四向向量
	 * @returns Vector2D 确保x, y绝对值都是1，长度为sqrt(2)
	 */
	[[nodiscard]] Vector2D getDiagonalFourWay() const noexcept {
		if (x > 0) {
			if (y > 0) return Vector2D(1, 1);
			return Vector2D(1, -1);
		}
		if (y < 0) return Vector2D(-1, -1);
		if (x < 0) return Vector2D(-1, 1);
		return Vector2D(1, 1); // (0, 1) => (1, 1)
	}

	[[nodiscard]] Vector2D nearestPointFrom(const Vector2D& point, const Vector2D& direction) const noexcept {
		const Vector2D normalized = direction.getNormalized();
		return point + normalized * clone().subtract(point).dot(normalized);
	}

	[[nodiscard]] Vector2D nearestPointFromNormalized(const Vector2D& point, const Vector2D& direction) const noexcept { return point + direction * clone().subtract(point).dot(direction); }

	[[nodiscard]] String toString() const noexcept { return L"(" + std::to_wstring(x) + L", " + std::to_wstring(y) + L")"; }
};

inline Vector2D operator*(const double scalar, const Vector2D& vector) noexcept { return vector * scalar; }
inline Vector3D operator*(const double scalar, const Vector3D& vector) noexcept { return vector * scalar; }
