//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\warnings.h"
#include "..\def.h"

inline constexpr double EpsilonValue = 1e-10;
inline constexpr double EpsilonCheck = 1e-8;

template <typename T> const T& nMin(const T& a, const T& b) { return a < b ? a : b; }

template <typename T> const T& nMin(const T& val0, const T& val1, const T& vals...) { return nMin(nMin(val0, val1), nMin(vals...)); }

template <typename T> const T& nMax(const T& a, const T& b) { return a > b ? a : b; }

template <typename T> const T& nMax(const T& val0, const T& val1, const T& vals...) { return nMax(nMax(val0, val1), nMax(vals...)); }

template <typename T> const T& nRange(const T& val, const T& min, const T& max) { return nMax(nMin(val, max), min); }

template <typename T> T nRangeSmooth(const T& val, const T& min, const T& max) {
	T dif = val - (min + max) * 0.5;
	T tanh_like = 1.0 - 1.0 / (1.0 + std::abs(dif));
	if (dif < 0) tanh_like = -tanh_like;
	return (min + max + tanh_like * (max - min)) * 0.5;
}

template <typename T, typename F> T nSlerp(const T& left, const T& right, const F& t) { return left + t * (right - left); }

template <typename T> bool nBetween(const T& val, const T& min, const T& max) { return val >= min && val <= max; }

template <typename T> bool nSideBetween(const T& val, const T& min, const T& max) { return min <= val && val < max; }

template <typename T> bool nSamePositivity(const T& val, const T& other) { return (val <= 0 && other <= 0) || (val >= 0 && other >= 0); }

template <typename T> consteval T nConsteval(T val) { return val; }

inline bool dEquals(const double v1, const double v2) noexcept { return std::abs(v1 - v2) < EpsilonCheck; }
inline bool dLess(const double v1, const double v2) noexcept { return v1 < v2 && !dEquals(v1, v2); }
inline bool dGreater(const double v1, const double v2) noexcept { return v1 > v2 && !dEquals(v1, v2); }
inline bool dLessEquals(const double v1, const double v2) noexcept { return v1 < v2 || dEquals(v1, v2); }
inline bool dGreaterEquals(const double v1, const double v2) noexcept { return v1 > v2 || dEquals(v1, v2); }

inline String dtoString(const double val) {
	std::wostringstream stream;
	stream << std::setprecision(std::numeric_limits<double>::digits10 + 1) << val;
	return stream.str();
}

template <typename T> void nMinMaxOf(QWORD& outMin, QWORD& outMax, const QWORD total, const T values[]) {
	outMin = outMax = 0;
	for (QWORD i = 0; i < total; ++i) {
		if (values[i] < values[outMin]) outMin = i;
		if (values[i] > values[outMax]) outMax = i;
	}
}

template <typename T> T nSquare(const T& val) { return val * val; }


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

	[[nodiscard]] double length() const noexcept {
		if (x == 0) {
			if (y == 0) return std::abs(z);
			if (z == 0) return std::abs(y);
			return sqrt(y * y + z * z);
		}
		if (y == 0) {
			if (z == 0) return std::abs(x);
			return sqrt(x * x + z * z);
		}
		if (z == 0) return std::sqrt(x * x + y * y);
		return std::sqrt(x * x + y * y + z * z);
	}

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

	Vector3D& strictSelect(const Vector3D& other) noexcept {
		if (other.x > 0) x = x > 0 ? nMin(x, other.x) : 0;
		else if (other.x < 0) x = x < 0 ? nMax(x, other.x) : 0;
		else x = 0;
		if (other.y > 0) y = y > 0 ? nMin(y, other.y) : 0;
		else if (other.y < 0) y = y < 0 ? nMax(y, other.y) : 0;
		else y = 0;
		if (other.z > 0) z = z > 0 ? nMin(z, other.z) : 0;
		else if (other.z < 0) z = z < 0 ? nMax(z, other.z) : 0;
		else z = 0;
		return *this;
	}

	[[nodiscard]] String toString() const noexcept { return L"Vector2D: (" + std::to_wstring(x) + L", " + std::to_wstring(y) + L", " + std::to_wstring(z) + L")"; }
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

	[[nodiscard]] double length() const noexcept {
		if (x == 0) return std::abs(y);
		if (y == 0) return std::abs(x);
		return std::sqrt(x * x + y * y);
	}

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

	Vector2D& strictSelect(const Vector2D& other) noexcept {
		if (other.x > 0) x = nRange(x, 0.0, other.x);
		else if (other.x < 0) x = nRange(x, other.x, 0.0);
		else x = 0;
		if (other.y > 0) y = nRange(y, 0.0, other.y);
		else if (other.y < 0) y = nRange(y, other.y, 0.0);
		else y = 0;
		return *this;
	}

	[[nodiscard]] String toString(const bool simplify = false) const noexcept { return simplify ? L"Vector3D: (" + std::to_wstring(x) + L", " + std::to_wstring(y) + L")" : L"Vector3D: (" + dtoString(x) + L", " + dtoString(y) + L")"; }
};

class Vector4D final {
	double x, y, z, w;

public:
	Vector4D(const double x, const double y, const double z, const double w) noexcept : x(x), y(y), z(z), w(w) {}
	Vector4D() noexcept : x(0), y(0), z(0), w(0) {}
	Vector4D(const Vector4D& other) noexcept = default;
	Vector4D(Vector4D&& other) noexcept = default;
	Vector4D& operator=(const Vector4D& other) noexcept = default;
	Vector4D& operator=(Vector4D&& other) noexcept = default;
	~Vector4D() noexcept = default;
	[[nodiscard]] double getX() const noexcept { return x; }
	[[nodiscard]] double getY() const noexcept { return y; }
	[[nodiscard]] double getZ() const noexcept { return z; }
	[[nodiscard]] double getW() const noexcept { return w; }
	[[nodiscard]] Vector4D clone() const noexcept { return Vector4D(x, y, z, w); }
	[[nodiscard]] Vector4D operator+(const Vector4D& other) const noexcept { return Vector4D(x + other.x, y + other.y, z + other.z, w + other.w); }
	[[nodiscard]] Vector4D operator-(const Vector4D& other) const noexcept { return Vector4D(x - other.x, y - other.y, z - other.z, w - other.w); }
	[[nodiscard]] Vector4D operator*(const double scalar) const noexcept { return Vector4D(x * scalar, y * scalar, z * scalar, w * scalar); }
	[[nodiscard]] Vector4D operator/(const double scalar) const noexcept { return Vector4D(x / scalar, y / scalar, z / scalar, w / scalar); }
	[[nodiscard]] Vector4D operator-() const noexcept { return Vector4D(-x, -y, -z, -w); }
	[[nodiscard]] double operator*(const Vector4D& other) const noexcept { return x * other.x + y * other.y + z * other.z + w * other.w; }
	[[nodiscard]] double lengthManhattan() const noexcept { return std::abs(x) + std::abs(y) + std::abs(z) + std::abs(w); }
	bool operator==(const Vector4D& other) const noexcept { return x == other.x && y == other.y && z == other.z && w == other.w; }
	bool operator!=(const Vector4D& other) const noexcept { return x != other.x || y != other.y || z != other.z || w != other.w; }
	Vector4D& operator+=(const Vector4D& other) noexcept { return x += other.x, y += other.y, z += other.z, w += other.w, *this; }
	Vector4D& operator-=(const Vector4D& other) noexcept { return x -= other.x, y -= other.y, z -= other.z, w -= other.w, *this; }
	Vector4D& operator*=(const double scalar) noexcept { return x *= scalar, y *= scalar, z *= scalar, w *= scalar, *this; }
	Vector4D& operator/=(const double scalar) noexcept { return x /= scalar, y /= scalar, z /= scalar, w /= scalar, *this; }
	Vector4D& add(const Vector4D& other) noexcept { return x += other.x, y += other.y, z += other.z, w += other.w, *this; }
	Vector4D& add(const double x, const double y, const double z, const double w) noexcept { return this->x += x, this->y += y, this->z += z, this->w += w, *this; }
	Vector4D& subtract(const Vector4D& other) noexcept { return x -= other.x, y -= other.y, z -= other.z, w -= other.w, *this; }
	Vector4D& subtract(const double x, const double y, const double z, const double w) noexcept { return this->x -= x, this->y -= y, this->z -= z, this->w -= w, *this; }
	Vector4D& multiply(const double value) noexcept { return x *= value, y *= value, z *= value, w *= value, *this; }
	Vector4D& divide(const double value) noexcept { return x /= value, y /= value, z /= value, w /= value, *this; }
	Vector4D& negate() noexcept { return x = -x, y = -y, z = -z, w = -w, *this; }
	Vector4D& setX(const double val) noexcept { return x = val, *this; }
	Vector4D& setY(const double val) noexcept { return y = val, *this; }
	Vector4D& setZ(const double val) noexcept { return z = val, *this; }
	Vector4D& setW(const double val) noexcept { return w = val, *this; }
	Vector4D& flipValueX() noexcept { return x = -x, *this; }
	Vector4D& flipValueY() noexcept { return y = -y, *this; }
	Vector4D& flipValueZ() noexcept { return z = -z, *this; }
	Vector4D& flipValueW() noexcept { return w = -w, *this; }
	[[nodiscard]] double dot(const Vector4D& other) const noexcept { return x * other.x + y * other.y + z * other.z + w * other.w; }
	[[nodiscard]] bool isZero() const noexcept { return x == 0 && y == 0 && z == 0 && w == 0; }
	[[nodiscard]] bool sameDirectionAs(const Vector4D& other) const noexcept { return isZero() || other.isZero() || dot(other) < other.length() * length() * DBL_EPSILON; }
	[[nodiscard]] double length() const noexcept { return std::sqrt(x * x + y * y + z * z + w * w); }

	[[nodiscard]] Vector4D getNormalized() const noexcept {
		if (isZero()) return Vector4D(0.0, 0.0, 0.0, 0.0);
		const double length = this->length();
		return Vector4D(x / length, y / length, z / length, w / length);
	}

	Vector4D& normalize() noexcept {
		if (isZero()) return x = 0, y = 0, z = 0, *this;
		const double length = this->length();
		x /= length, y /= length, z /= length, w /= length;
		return *this;
	}

	[[nodiscard]] String toString() const noexcept { return L"Vector4D: (" + std::to_wstring(x) + L", " + std::to_wstring(y) + L", " + std::to_wstring(z) + L", " + std::to_wstring(w) + L")"; }
};

inline Vector2D operator*(const double scalar, const Vector2D& vector) noexcept { return vector * scalar; }
inline Vector3D operator*(const double scalar, const Vector3D& vector) noexcept { return vector * scalar; }
inline Vector4D operator*(const double scalar, const Vector4D& vector) noexcept { return vector * scalar; }

class Quaternion final {
public:
	double x = 0, y = 0, z = 0, w = 0;
};

class Matrix3D {
public:
	double a11 = 0, a12 = 0, a13 = 0;
	double a21 = 0, a22 = 0, a23 = 0;
	double a31 = 0, a32 = 0, a33 = 0;

	Matrix3D() = default;
	Matrix3D(const Matrix3D& other) = default;
	Matrix3D& operator=(const Matrix3D& other) = default;

	Matrix3D(const double a11, const double a12, const double a13, const double a21, const double a22, const double a23, const double a31, const double a32, const double a33) noexcept {
		this->a11 = a11, this->a12 = a12, this->a13 = a13;
		this->a21 = a21, this->a22 = a22, this->a23 = a23;
		this->a31 = a31, this->a32 = a32, this->a33 = a33;
	}

	[[nodiscard]] Matrix3D operator+(const Matrix3D& other) const noexcept { return Matrix3D(a11 + other.a11, a12 + other.a12, a13 + other.a13, a21 + other.a21, a22 + other.a22, a23 + other.a23, a31 + other.a31, a32 + other.a32, a33 + other.a33); }
	[[nodiscard]] Matrix3D operator-(const Matrix3D& other) const noexcept { return Matrix3D(a11 - other.a11, a12 - other.a12, a13 - other.a13, a21 - other.a21, a22 - other.a22, a23 - other.a23, a31 - other.a31, a32 - other.a32, a33 - other.a33); }
	[[nodiscard]] Matrix3D operator*(const double value) const noexcept { return Matrix3D(a11 * value, a12 * value, a13 * value, a21 * value, a22 * value, a23 * value, a31 * value, a32 * value, a33 * value); }
	[[nodiscard]] Vector3D operator*(const Vector3D& vector) const noexcept { return Vector3D(a11 * vector.getX() + a12 * vector.getY() + a13 * vector.getZ(), a21 * vector.getX() + a22 * vector.getY() + a23 * vector.getZ(), a31 * vector.getX() + a32 * vector.getY() + a33 * vector.getZ()); }
	[[nodiscard]] Matrix3D operator-() const noexcept { return Matrix3D(-a11, -a12, -a13, -a21, -a22, -a23, -a31, -a32, -a33); }
	[[nodiscard]] Matrix3D& operator+=(const Matrix3D& other) noexcept { return a11 += other.a11, a12 += other.a12, a13 += other.a13, a21 += other.a21, a22 += other.a22, a23 += other.a23, a31 += other.a31, a32 += other.a32, a33 += other.a33, *this; }
	[[nodiscard]] Matrix3D& operator-=(const Matrix3D& other) noexcept { return a11 -= other.a11, a12 -= other.a12, a13 -= other.a13, a21 -= other.a21, a22 -= other.a22, a23 -= other.a23, a31 -= other.a31, a32 -= other.a32, a33 -= other.a33, *this; }
	[[nodiscard]] Matrix3D& operator*=(const Matrix3D& other) noexcept { return *this = operator*(other); }
	[[nodiscard]] Matrix3D& operator*=(const double value) noexcept { return a11 *= value, a12 *= value, a13 *= value, a21 *= value, a22 *= value, a23 *= value, a31 *= value, a32 *= value, a33 *= value, *this; }
	[[nodiscard]] Matrix3D& add(const Matrix3D& other) noexcept { return operator+=(other); }
	[[nodiscard]] Matrix3D& subtract(const Matrix3D& other) noexcept { return operator-=(other); }
	[[nodiscard]] Matrix3D& multiply(const double value) noexcept { return operator*=(value); }
	[[nodiscard]] Matrix3D& multiply(const Matrix3D& other) noexcept { return operator*=(other); }
	[[nodiscard]] Vector3D multiply(const Vector3D& vector) const noexcept { return operator*(vector); }
	[[nodiscard]] Matrix3D& negate() noexcept { return a11 = -a11, a12 = -a12, a13 = -a13, a21 = -a21, a22 = -a22, a23 = -a23, a31 = -a31, a32 = -a32, a33 = -a33, *this; }

	[[nodiscard]] Matrix3D operator*(const Matrix3D& other) const noexcept {
		return Matrix3D(
			a11 * other.a11 + a12 * other.a21 + a13 * other.a31,
			a11 * other.a12 + a12 * other.a22 + a13 * other.a32,
			a11 * other.a13 + a12 * other.a23 + a13 * other.a33,
			a21 * other.a11 + a22 * other.a21 + a23 * other.a31,
			a21 * other.a12 + a22 * other.a22 + a23 * other.a32,
			a21 * other.a13 + a22 * other.a23 + a23 * other.a33,
			a31 * other.a11 + a32 * other.a21 + a33 * other.a31,
			a31 * other.a12 + a32 * other.a22 + a33 * other.a32,
			a31 * other.a13 + a32 * other.a23 + a33 * other.a33
		);
	}
};

class Matrix4D {
public:
	double a11 = 0, a12 = 0, a13 = 0, a14 = 0;
	double a21 = 0, a22 = 0, a23 = 0, a24 = 0;
	double a31 = 0, a32 = 0, a33 = 0, a34 = 0;
	double a41 = 0, a42 = 0, a43 = 0, a44 = 0;

	Matrix4D() = default;
	Matrix4D(const Matrix4D& other) = default;
	Matrix4D& operator=(const Matrix4D& other) = default;
	Matrix4D(const Matrix3D& other) { operator=(other); }

	Matrix4D(const double a11, const double a12, const double a13, const double a14, const double a21, const double a22, const double a23, const double a24, const double a31, const double a32, const double a33, const double a34, const double a41, const double a42, const double a43, const double a44) noexcept {
		this->a11 = a11, this->a12 = a12, this->a13 = a13, this->a14 = a14;
		this->a21 = a21, this->a22 = a22, this->a23 = a23, this->a24 = a24;
		this->a31 = a31, this->a32 = a32, this->a33 = a33, this->a34 = a34;
		this->a41 = a41, this->a42 = a42, this->a43 = a43, this->a44 = a44;
	}

	Matrix4D& operator=(const Matrix3D& other) {
		a11 = other.a11, a12 = other.a12, a13 = other.a13, a14 = 0;
		a21 = other.a21, a22 = other.a22, a23 = other.a23, a24 = 0;
		a31 = other.a31, a32 = other.a32, a33 = other.a33, a34 = 0;
		a41 = 0, a42 = 0, a43 = 0, a44 = 1;
		return *this;
	}

	[[nodiscard]] Matrix4D operator+(const Matrix4D& other) const noexcept { return Matrix4D(a11 + other.a11, a12 + other.a12, a13 + other.a13, a14 + other.a14, a21 + other.a21, a22 + other.a22, a23 + other.a23, a24 + other.a24, a31 + other.a31, a32 + other.a32, a33 + other.a33, a34 + other.a34, a41 + other.a41, a42 + other.a42, a43 + other.a43, a44 + other.a44); }
	[[nodiscard]] Matrix4D operator-(const Matrix4D& other) const noexcept { return Matrix4D(a11 - other.a11, a12 - other.a12, a13 - other.a13, a14 - other.a14, a21 - other.a21, a22 - other.a22, a23 - other.a23, a24 - other.a24, a31 - other.a31, a32 - other.a32, a33 - other.a33, a34 - other.a34, a41 - other.a41, a42 - other.a42, a43 - other.a43, a44 - other.a44); }
	[[nodiscard]] Matrix4D operator*(const double value) const noexcept { return Matrix4D(a11 * value, a12 * value, a13 * value, a14 * value, a21 * value, a22 * value, a23 * value, a24 * value, a31 * value, a32 * value, a33 * value, a34 * value, a41 * value, a42 * value, a43 * value, a44 * value); }
	[[nodiscard]] Vector4D operator*(const Vector4D& vec) const noexcept { return Vector4D(a11 * vec.getX() + a12 * vec.getY() + a13 * vec.getZ() + a14 * vec.getW(), a21 * vec.getX() + a22 * vec.getY() + a23 * vec.getZ() + a24 * vec.getW(), a31 * vec.getX() + a32 * vec.getY() + a33 * vec.getZ() + a34 * vec.getW(), a41 * vec.getX() + a42 * vec.getY() + a43 * vec.getZ() + a44 * vec.getW()); }
	[[nodiscard]] Matrix4D operator-() const noexcept { return Matrix4D(-a11, -a12, -a13, -a14, -a21, -a22, -a23, -a24, -a31, -a32, -a33, -a34, -a41, -a42, -a43, -a44); }
	[[nodiscard]] Matrix4D& operator+=(const Matrix4D& other) noexcept { return a11 += other.a11, a12 += other.a12, a13 += other.a13, a14 += other.a14, a21 += other.a21, a22 += other.a22, a23 += other.a23, a24 += other.a24, a31 += other.a31, a32 += other.a32, a33 += other.a33, a34 += other.a34, a41 += other.a41, a42 += other.a42, a43 += other.a43, a44 += other.a44, *this; }
	[[nodiscard]] Matrix4D& operator-=(const Matrix4D& other) noexcept { return a11 -= other.a11, a12 -= other.a12, a13 -= other.a13, a14 -= other.a14, a21 -= other.a21, a22 -= other.a22, a23 -= other.a23, a24 -= other.a24, a31 -= other.a31, a32 -= other.a32, a33 -= other.a33, a34 -= other.a34, a41 -= other.a41, a42 -= other.a42, a43 -= other.a43, a44 -= other.a44, *this; }
	[[nodiscard]] Matrix4D& operator*=(const Matrix4D& other) noexcept { return *this = operator*(other); }
	[[nodiscard]] Matrix4D& operator*=(const double value) noexcept { return a11 *= value, a12 *= value, a13 *= value, a14 *= value, a21 *= value, a22 *= value, a23 *= value, a24 *= value, a31 *= value, a32 *= value, a33 *= value, a34 *= value, a41 *= value, a42 *= value, a43 *= value, a44 *= value, *this; }
	[[nodiscard]] Matrix4D& add(const Matrix4D& other) noexcept { return operator+=(other); }
	[[nodiscard]] Matrix4D& subtract(const Matrix4D& other) noexcept { return operator-=(other); }
	[[nodiscard]] Matrix4D& multiply(const double value) noexcept { return operator*=(value); }
	[[nodiscard]] Matrix4D& multiply(const Matrix4D& other) noexcept { return operator*=(other); }
	[[nodiscard]] Vector4D multiply(const Vector4D& vector) const noexcept { return operator*(vector); }
	[[nodiscard]] Matrix4D& negate() noexcept { return a11 = -a11, a12 = -a12, a13 = -a13, a14 = -a14, a21 = -a21, a22 = -a22, a23 = -a23, a24 = -a24, a31 = -a31, a32 = -a32, a33 = -a33, a34 = -a34, a41 = -a41, a42 = -a42, a43 = -a43, a44 = -a44, *this; }

	[[nodiscard]] Matrix4D operator*(const Matrix4D& other) const noexcept {
		return Matrix4D(
			a11 * other.a11 + a12 * other.a21 + a13 * other.a31 + a14 * other.a41,
			a11 * other.a12 + a12 * other.a22 + a13 * other.a32 + a14 * other.a42,
			a11 * other.a13 + a12 * other.a23 + a13 * other.a33 + a14 * other.a43,
			a11 * other.a14 + a12 * other.a24 + a13 * other.a34 + a14 * other.a44,
			a21 * other.a11 + a22 * other.a21 + a23 * other.a31 + a24 * other.a41,
			a21 * other.a12 + a22 * other.a22 + a23 * other.a32 + a24 * other.a42,
			a21 * other.a13 + a22 * other.a23 + a23 * other.a33 + a24 * other.a43,
			a21 * other.a14 + a22 * other.a24 + a23 * other.a34 + a24 * other.a44,
			a31 * other.a11 + a32 * other.a21 + a33 * other.a31 + a34 * other.a41,
			a31 * other.a12 + a32 * other.a22 + a33 * other.a32 + a34 * other.a42,
			a31 * other.a13 + a32 * other.a23 + a33 * other.a33 + a34 * other.a43,
			a31 * other.a14 + a32 * other.a24 + a33 * other.a34 + a34 * other.a44,
			a41 * other.a11 + a42 * other.a21 + a43 * other.a31 + a44 * other.a41,
			a41 * other.a12 + a42 * other.a22 + a43 * other.a32 + a44 * other.a42,
			a41 * other.a13 + a42 * other.a23 + a43 * other.a33 + a44 * other.a43,
			a41 * other.a14 + a42 * other.a24 + a43 * other.a34 + a44 * other.a44
		);
	}
};
