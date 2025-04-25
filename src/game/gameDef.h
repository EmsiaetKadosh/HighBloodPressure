//
// Created by EmsiaetKadosh on 25-3-30.
//

#pragma once

class Entity;
class RayTraceBlockResult;
class RayTraceEntityResult;
struct RayTraceResults;
class BoundingBoxCollideBlockResult;
class BoundingBoxCollideEntityResult;
struct BoundingBoxCollideResults;

class [[carlbeks::defineat("World.h")]] World;
class [[carlbeks::defineat("World.h")]] WorldManager;

struct CollidingSide {
	using Enum = enum : char {
		LEFT_BOTTOM = -4,
		BOTTOM = -3,
		RIGHT_BOTTOM = -2,
		RIGHT = -1,
		COVER = 0,
		LEFT = 1,
		LEFT_TOP = 2,
		TOP = 3,
		RIGHT_TOP = 4,
	};
	char value = COVER;
	inline static const Vector2D DIRECTIONAL[]{
		Vector2D(-0.7071067811865475, 0.7071067811865475),
		Vector2D(0, 1),
		Vector2D(0.7071067811865475, 0.7071067811865475),
		Vector2D(1, 0),
		Vector2D(),
		Vector2D(-1, 0),
		Vector2D(-0.7071067811865475, -0.7071067811865475),
		Vector2D(0, -1),
		Vector2D(0.7071067811865475, -0.7071067811865475),

		Vector2D(-1, 1),
		Vector2D(0, 1),
		Vector2D(1, 1),
		Vector2D(1, 0),
		Vector2D(),
		Vector2D(-1, 0),
		Vector2D(-1, -1),
		Vector2D(0, -1),
		Vector2D(1, -1)
	};

	CollidingSide() = default;
	CollidingSide(const Enum value) : value(value) {}
	CollidingSide(const CollidingSide& other) noexcept = default;
	CollidingSide& operator=(const CollidingSide& other) noexcept = default;
	CollidingSide& reverse() noexcept { return value = static_cast<char>(-value), *this; }
	[[nodiscard]] operator Enum() const noexcept { return static_cast<Enum>(value); }
	[[nodiscard]] bool operator==(const Enum& other) const noexcept { return value == other; }
	[[nodiscard]] bool operator==(const CollidingSide& other) const noexcept { return value == other.value; }
	[[nodiscard]] bool operator!=(const Enum& other) const noexcept { return value != other; }
	[[nodiscard]] bool operator!=(const CollidingSide& other) const noexcept { return value != other.value; }
	[[nodiscard]] CollidingSide getReversed() const noexcept { return CollidingSide(*this).reverse(); }
	[[nodiscard]] Vector2D getDirectionNormalized() const noexcept { return DIRECTIONAL[value + 4]; }
	[[nodiscard]] Vector2D getDirectionBlock() const noexcept { return DIRECTIONAL[value + 13]; }

	[[nodiscard]] String toString() const noexcept {
		switch (value) {
			case COVER:
				return L"Cover";
			case LEFT:
				return L"Left";
			case RIGHT:
				return L"Right";
			case BOTTOM:
				return L"Bottom";
			case TOP:
				return L"Top";
			case LEFT_TOP:
				return L"LeftTop";
			case RIGHT_TOP:
				return L"RightTop";
			case LEFT_BOTTOM:
				return L"LeftBottom";
			case RIGHT_BOTTOM:
				return L"RightBottom";
			default:
				return L"Unknown";
		}
	}

	[[nodiscard]] CollidingSide getClockwiseRotated() const noexcept {
		if (value > 0) {
			if (value == 4) return CollidingSide(static_cast<Enum>(-1));
			return CollidingSide(static_cast<Enum>(value + 1));
		}
		if (value < 0) {
			if (value == -4) return CollidingSide(static_cast<Enum>(1));
			return CollidingSide(static_cast<Enum>(value - 1));
		}
		return CollidingSide(static_cast<Enum>(0));
	}

	[[nodiscard]] CollidingSide getAntiClockwiseRotated() const noexcept {
		if (value > 0) {
			if (value == 1) return CollidingSide(static_cast<Enum>(-4));
			return CollidingSide(static_cast<Enum>(value - 1));
		}
		if (value < 0) {
			if (value == -1) return CollidingSide(static_cast<Enum>(4));
			return CollidingSide(static_cast<Enum>(value + 1));
		}
		return CollidingSide(static_cast<Enum>(0));
	}

	[[nodiscard]] static CollidingSide fromVector2D(const Vector2D& vector) {
		if (vector.getX() > 0) {
			if (vector.getY() > vector.getX()) return BOTTOM;
			if (vector.getY() == vector.getX()) return RIGHT_BOTTOM;
			if (vector.getY() < -vector.getX()) return TOP;
			if (vector.getY() == -vector.getX()) return RIGHT_TOP;
			return RIGHT;
		}
		if (vector.getX() == 0) {
			if (vector.getY() > 0) return BOTTOM;
			if (vector.getY() < 0) return TOP;
			return COVER;
		}
		if (vector.getY() < vector.getX()) return TOP;
		if (vector.getY() == vector.getX()) return LEFT_TOP;
		if (vector.getY() > -vector.getX()) return BOTTOM;
		if (vector.getY() == -vector.getX()) return LEFT_BOTTOM;
		return LEFT;
	}
};

class RayTraceBlockResult {
	BlockLocation location;
	Vector2D hitPoint;
	CollidingSide hitSide;

public:
	RayTraceBlockResult(const BlockLocation& location, const Vector2D& hitPoint, const CollidingSide& hitSide) : location(location), hitPoint(hitPoint), hitSide(hitSide) {}
	[[nodiscard]] const BlockLocation& getBlockLocation() const noexcept { return location; }
	[[nodiscard]] const Vector2D& getHitPoint() const noexcept { return hitPoint; }
	[[nodiscard]] const CollidingSide& getHitSide() const noexcept { return hitSide; }
};

class RayTraceEntityResult {
	Entity* entity;
	Vector2D hitPoint;
	CollidingSide hitSide;

public:
	RayTraceEntityResult(Entity* entity, const Vector2D& hitPoint, const CollidingSide& hitSide) : entity(entity), hitPoint(hitPoint), hitSide(hitSide) {}
};

struct RayTraceResults {
	Vector<RayTraceBlockResult> locations;
	Vector<RayTraceEntityResult> entities;
};

class BoundingBoxCollideBlockResult {
	friend class World;
	BlockLocation block; // 8
	unsigned int order; // 4
	CollidingSide side; // 1
public:
	BoundingBoxCollideBlockResult(const BlockLocation& block, unsigned int order, const CollidingSide collidingSide) : block(block), order(order), side(collidingSide) {}
	[[nodiscard]] const BlockLocation& getBlockLocation() const noexcept { return block; }
	[[nodiscard]] unsigned int getOrder() const noexcept { return order; }
	[[nodiscard]] CollidingSide getCollidingSide() const noexcept { return side; }

	struct Less {
		[[nodiscard]] bool operator()(const BoundingBoxCollideBlockResult& lhs, const BoundingBoxCollideBlockResult& rhs) const noexcept {
			if (lhs.order < rhs.order) return true;
			if (lhs.order > rhs.order) return false;
			return BlockLocation::Less()(lhs.block, rhs.block);
		}
	};
};

class BoundingBoxCollideEntityResult {
	Entity* entity;
	CollidingSide side;

public:
	BoundingBoxCollideEntityResult(Entity* entity, const CollidingSide side) : entity(entity), side(side) {}
};

struct BoundingBoxCollideResults {
	Set<BoundingBoxCollideBlockResult, BoundingBoxCollideBlockResult::Less> blocks;
	Vector<BoundingBoxCollideEntityResult> entities;
};
namespace $LimitedAccess {
	struct BoundingBoxTraceOrder {
		double val;
		unsigned int order;
	};

	struct BoundingBoxTraceLessX {
		using is_transparent = BoundingBoxCollideBlockResult;
		[[nodiscard]] bool operator()(const BoundingBoxTraceOrder& left, const BoundingBoxTraceOrder& right) const { return left.order < right.order; }

		[[nodiscard]] bool operator()(const BoundingBoxCollideBlockResult& left, const BoundingBoxTraceOrder& right) const {
			switch (left.getCollidingSide()) {
				case CollidingSide::LEFT:
				case CollidingSide::LEFT_TOP:
				case CollidingSide::LEFT_BOTTOM:
					return std::round(left.getBlockLocation().getX()) < right.val;
				case CollidingSide::RIGHT_TOP:
				case CollidingSide::RIGHT:
				case CollidingSide::RIGHT_BOTTOM:
					return std::round(left.getBlockLocation().getX() + 1) > right.val;
				default:
					unreachable();
			}
		}

		[[nodiscard]] bool operator()(const BoundingBoxTraceOrder& left, const BoundingBoxCollideBlockResult& right) const {
			switch (right.getCollidingSide()) {
				case CollidingSide::LEFT:
				case CollidingSide::LEFT_TOP:
				case CollidingSide::LEFT_BOTTOM:
					return left.val < std::round(right.getBlockLocation().getX());
				case CollidingSide::RIGHT:
				case CollidingSide::RIGHT_TOP:
				case CollidingSide::RIGHT_BOTTOM:
					return left.val > std::round(right.getBlockLocation().getX() + 1);
				default:
					unreachable();
			}
		}
	};

	struct BoundingBoxTraceLessY {
		using is_transparent = BoundingBoxCollideBlockResult;
		[[nodiscard]] bool operator()(const BoundingBoxTraceOrder& left, const BoundingBoxTraceOrder& right) const { return left.order < right.order; }

		[[nodiscard]] bool operator()(const BoundingBoxCollideBlockResult& left, const BoundingBoxTraceOrder& right) const {
			switch (left.getCollidingSide()) {
				case CollidingSide::TOP:
				case CollidingSide::LEFT_TOP:
				case CollidingSide::RIGHT_TOP:
					return std::round(left.getBlockLocation().getY()) < right.val;
				case CollidingSide::BOTTOM:
				case CollidingSide::LEFT_BOTTOM:
				case CollidingSide::RIGHT_BOTTOM:
					return std::round(left.getBlockLocation().getY() + 1) > right.val;
				default:
					unreachable();
			}
		}

		[[nodiscard]] bool operator()(const BoundingBoxTraceOrder& left, const BoundingBoxCollideBlockResult& right) const {
			switch (right.getCollidingSide()) {
				case CollidingSide::TOP:
				case CollidingSide::LEFT_TOP:
				case CollidingSide::RIGHT_TOP:
					return left.val < std::round(right.getBlockLocation().getY());
				case CollidingSide::BOTTOM:
				case CollidingSide::LEFT_BOTTOM:
				case CollidingSide::RIGHT_BOTTOM:
					return left.val > std::round(right.getBlockLocation().getY() + 1);
				default:
					unreachable();
			}
		}
	};
}

