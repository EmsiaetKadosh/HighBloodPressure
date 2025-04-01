//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\..\utils\math.h"

using WorldID = QWORD;
class Location;
class BlockLocation;
class [[carlbeks::predecl, carlbeks::defineat("World.h")]] World;

/**
 * 当一个方块/实体被移入/移出世界时，需要有一个理由。
 * 如果方块移出时附带内置的Shutdown理由，那么方块在移出时会被立刻自动submit至gc。
 *   Shutdown理由的含义是，世界管理器下的所有世界内的所有方块，都会被依次附带Shutdown理由移出世界。
 *   此过程中不可以进行其他的方块WorldTransport行为（因为正在遍历方块），也不需要手动清除其他的关联方块。
 *   禁止在处理附带Shutdown理由的移出世界事件时提交自身给gc。但是方块自身的其他垃圾可能需要手动清理。
 *
 * 该类视同enum类。元素只允许引用访问，不允许复制、移动。
 */
class WorldTransportReason final {
	friend class World;
	struct LessReason;

	class Reason final {
		friend struct LessReason;
		friend class WorldTransportReason;
		const String description;
		const bool isBlockReason, isEntityReason;
		~Reason() = default;

	public:
		Reason(const String& description, const bool isBlockReason, const bool isEntityReason) : description(description), isBlockReason(isBlockReason), isEntityReason(isEntityReason) {}
		Reason(String&& description, const bool isBlockReason, const bool isEntityReason) : description(std::move(description)), isBlockReason(isBlockReason), isEntityReason(isEntityReason) {}
		Reason(const Reason&) = delete;
		Reason(Reason&&) = delete;
		Reason& operator=(const Reason&) = delete;
		Reason& operator=(Reason&&) = delete;
	};

	struct LessReason {
		using is_transparent = String;
		[[nodiscard]] bool operator()(const Reason* const lhs, const Reason* const rhs) const { return lhs->description < rhs->description; }
		[[nodiscard]] bool operator()(const String& str, const Reason* const rhs) const { return str < rhs->description; }
		[[nodiscard]] bool operator()(const Reason* const lhs, const String& str) const { return lhs->description < str; }
	};

	struct Manager {
		Set<Reason*, LessReason> reasons;
		~Manager() { for (Reason* r : reasons) delete deallocating(r); }

		Reason* create(const String& description, const bool isBlockReason, const bool isEntityReason) {
			if (const Set<Reason*, LessReason>::const_iterator iter = reasons.find(description); iter != reasons.end()) return iter.operator*();
			Reason* r = allocatedFor(new Reason(description, isBlockReason, isEntityReason));
			reasons.emplace(r);
			return r;
		}

		Reason* create(String&& description, const bool isBlockReason, const bool isEntityReason) {
			if (const Set<Reason*, LessReason>::const_iterator iter = reasons.find(description); iter != reasons.end()) return iter.operator*();
			Reason* r = allocatedFor(new Reason(std::move(description), isBlockReason, isEntityReason));
			reasons.emplace(r);
			return r;
		}
	} inline static manager;

	static const WorldTransportReason Shutdown;
	const Reason* reason;
	WorldTransportReason(const Reason& reason) : reason(&reason) {}

public:
	static const WorldTransportReason InitialGeneration;
	static const WorldTransportReason WorldCollapse;
	static const WorldTransportReason BlockBreak;
	static const WorldTransportReason BlockReplace;
	static const WorldTransportReason EntityTeleport;

	WorldTransportReason() = delete;
	WorldTransportReason(const WorldTransportReason&) = default;
	WorldTransportReason(WorldTransportReason&&) = default;
	WorldTransportReason& operator=(const WorldTransportReason&) = default;
	WorldTransportReason& operator=(WorldTransportReason&&) = default;
	~WorldTransportReason() = default;
	[[nodiscard]] bool operator==(const WorldTransportReason& other) const noexcept { return reason == other.reason; }
	[[nodiscard]] bool operator!=(const WorldTransportReason& other) const noexcept { return reason != other.reason; }
	[[nodiscard]] bool isBlockReason() const noexcept { return reason->isBlockReason; }
	[[nodiscard]] bool isEntityReason() const noexcept { return reason->isEntityReason; }

	static WorldTransportReason registerReason(const String& description, const bool isBlockReason, const bool isEntityReason) { return *manager.create(description, isBlockReason, isEntityReason); }
	static WorldTransportReason registerReason(String&& description, const bool isBlockReason, const bool isEntityReason) { return *manager.create(std::move(description), isBlockReason, isEntityReason); }
};

class [[carlbeks::TriviallyCopyable]] Location {
	Vector2D position;
	WorldID idWorld;

public:
	Location(const Vector2D& position) noexcept : position(position), idWorld(0) {}
	Location(const Vector2D& position, const WorldID idWorld) noexcept : position(position), idWorld(idWorld) {}
	Location(const Location& other) noexcept = default;
	Location(Location&& other) noexcept = default;
	[[nodiscard]] Vector2D getPosition() const noexcept { return position; }
	[[nodiscard]] WorldID getWorld() const noexcept { return idWorld; }
	[[nodiscard]] double getX() const noexcept { return position.getX(); }
	[[nodiscard]] double getY() const noexcept { return position.getY(); }
	[[nodiscard]] BlockLocation toBlockLocation() const noexcept;
	void setPosition(const Vector2D& vector) noexcept { position = vector; }
	void setWorld(const WorldID idWorld) noexcept { this->idWorld = idWorld; }
};

class [[carlbeks::TriviallyCopyable]] BlockLocation {
	long long x, y;
	WorldID idWorld;

public:
	BlockLocation(const long long x, const long long y) noexcept : x(x), y(y), idWorld(0) {}
	BlockLocation(const Vector2D& position) noexcept : x(static_cast<long long>(std::floor(position.getX()))), y(static_cast<long long>(std::floor(position.getY()))), idWorld(0) {}
	BlockLocation(const long long x, const long long y, const WorldID idWorld) noexcept : x(x), y(y), idWorld(idWorld) {}
	BlockLocation(const Vector2D& position, const WorldID idWorld) noexcept : x(static_cast<long long>(std::floor(position.getX()))), y(static_cast<long long>(std::floor(position.getY()))), idWorld(idWorld) {}
	BlockLocation(const BlockLocation& other) noexcept = default;
	BlockLocation(BlockLocation&& other) noexcept = default;
	[[nodiscard]] Vector2D getPosition() const noexcept { return Vector2D(static_cast<double>(x), static_cast<double>(y)); }
	[[nodiscard]] WorldID getWorld() const noexcept { return idWorld; }
	[[nodiscard]] long long getX() const noexcept { return x; }
	[[nodiscard]] long long getY() const noexcept { return y; }
	[[nodiscard]] Location toLocation() const noexcept { return Location({static_cast<double>(x), static_cast<double>(y)}, idWorld); }
	void setPosition(const long long x, const long long y) noexcept { this->x = x, this->y = y; }
	void setWorld(const WorldID idWorld) noexcept { this->idWorld = idWorld; }
	[[nodiscard]] String toString() const { return L"(" + std::to_wstring(x) + L", " + std::to_wstring(y) + L") @ " + std::to_wstring(idWorld); }

	[[nodiscard]] static bool blockContains(const Vector2D& position, const Vector2D& other) { return position.getX() <= other.getX() && position.getY() <= other.getY() && position.getX() + 1.0 >= other.getX() && position.getY() + 1.0 >= other.getY(); }
	[[nodiscard]] static bool blockCompletelyContains(const Vector2D& position, const Vector2D& other) { return position.getX() < other.getX() && position.getY() < other.getY() && position.getX() + 1.0 > other.getX() && position.getY() + 1.0 > other.getY(); }
	[[nodiscard]] static bool blockCenterContains(const Vector2D& center, const Vector2D& other) { return center.getX() - 0.5 <= other.getX() && center.getY() - 0.5 <= other.getY() && center.getX() + 0.5 >= other.getX() && center.getY() + 0.5 >= other.getY(); }

	struct Less {
		bool operator()(const BlockLocation& lhs, const BlockLocation& rhs) const noexcept;
	};
};

inline bool BlockLocation::Less::operator()(const BlockLocation& lhs, const BlockLocation& rhs) const noexcept { return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x); }

inline BlockLocation Location::toBlockLocation() const noexcept { return BlockLocation(position, idWorld); }
