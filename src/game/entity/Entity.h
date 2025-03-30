//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\..\utils\math.h"
#include "..\..\render\Renderer.h"
#include "..\world\Location.h"
#include "Damage.h"

/*
 * 实体结构设计
 * 攻击时，依次执行：
 *   来源攻击（熔断命中判定）
 *   目标被命中（熔断命中判定）
 *   来源计算伤害（来源装备、buff计算）
 *   目标计算伤害（目标装备、buff计算）
 *   目标格挡判定（熔断格挡）
 *   （熔断伤害零值判断）
 *   目标应用、受到伤害（护盾判断）
 *   来源造成伤害后（吸血应用）
 *   目标受到伤害后（弹反等，应该是提交Task应用，或者原地调用，看实现和调用栈情况）
 */

class Entity;
class [[carlbeks::predecl, carlbeks::defineat("Player.h")]] Player;
class [[carlbeks::predecl, carlbeks::defineat("World.h")]] World;
class EntityManager;

using EntityID = QWORD;

class BoundingBox {
	// 分别表示，相对于实体位置，碰撞箱的四个方向的偏移分别是多少
	double left = 0.5, right = 0.5, top = 1, bottom = 0;

public:
	BoundingBox() noexcept = default;
	BoundingBox(const double left, const double right, const double top, const double bottom) noexcept : left(left), right(right), top(top), bottom(bottom) {}
	void setLeft(const double value) noexcept { left = value; }
	void setRight(const double value) noexcept { right = value; }
	void setTop(const double value) noexcept { top = value; }
	void setBottom(const double value) noexcept { bottom = value; }
	[[nodiscard]] double getLeft() const noexcept { return left; }
	[[nodiscard]] double getRight() const noexcept { return right; }
	[[nodiscard]] double getTop() const noexcept { return top; }
	[[nodiscard]] double getBottom() const noexcept { return bottom; }
	[[nodiscard]] double getWidth() const noexcept { return right + left; }
	[[nodiscard]] double getHeight() const noexcept { return bottom + top; }
	[[nodiscard]] Vector2D getCenterOffset() const noexcept { return Vector2D(left + right, top + bottom); }
	[[nodiscard]] Vector2D getLeftTopOffset() const noexcept { return Vector2D(-left, -top); }
	[[nodiscard]] Vector2D getRightTopOffset() const noexcept { return Vector2D(right, -top); }
	[[nodiscard]] Vector2D getLeftBottomOffset() const noexcept { return Vector2D(-left, bottom); }
	[[nodiscard]] Vector2D getRightBottomOffset() const noexcept { return Vector2D(right, bottom); }

	void getFarthestOffset(Vector2D& outPointPositive, Vector2D& outPointNegative, const Vector2D& direction) const noexcept {
		QWORD min, max;
		const double values[] = {
			direction.cross(getLeftTopOffset()).getZ(),
			direction.cross(getRightTopOffset()).getZ(),
			direction.cross(getLeftBottomOffset()).getZ(),
			direction.cross(getRightBottomOffset()).getZ()
		};
		nMinMaxOf<double>(min, max, 4, values);
		switch (max) {
			case 0:
				outPointPositive = getLeftTopOffset();
				break;
			case 1:
				outPointPositive = getRightTopOffset();
				break;
			case 2:
				outPointPositive = getLeftBottomOffset();
				break;
			case 3:
				outPointPositive = getRightBottomOffset();
				break;
			default:
				unreachable();
		}
		switch (min) {
			case 0:
				outPointNegative = getLeftTopOffset();
				break;
			case 1:
				outPointNegative = getRightTopOffset();
				break;
			case 2:
				outPointNegative = getLeftBottomOffset();
				break;
			case 3:
				outPointNegative = getRightBottomOffset();
				break;
			default:
				unreachable();
		}
	}

	void getForefrontOffset(Vector2D& outPointForward, Vector2D& outPointBackward, const Vector2D& direction) const noexcept {
		QWORD min, max;
		const double values[] = {
			direction.dot(getLeftTopOffset()),
			direction.dot(getRightTopOffset()),
			direction.dot(getLeftBottomOffset()),
			direction.dot(getRightBottomOffset())
		};
		nMinMaxOf<double>(min, max, 4, values);
		switch (max) {
			case 0:
				outPointForward = getLeftTopOffset();
				break;
			case 1:
				outPointForward = getRightTopOffset();
				break;
			case 2:
				outPointForward = getLeftBottomOffset();
				break;
			case 3:
				outPointForward = getRightBottomOffset();
				break;
			default:
				unreachable();
		}
		switch (min) {
			case 0:
				outPointBackward = getLeftTopOffset();
				break;
			case 1:
				outPointBackward = getRightTopOffset();
				break;
			case 2:
				outPointBackward = getLeftBottomOffset();
				break;
			case 3:
				outPointBackward = getRightBottomOffset();
				break;
			default:
				unreachable();
		}
	}

	/**
	 * 获取在position处该碰撞箱直接覆盖到的方块。BlockLocation对应的范围是[return.left, return.right); [return.top, return.bottom)
	 */
	[[nodiscard]] RECT getCoveringBlocks(const Vector2D& position) const noexcept {
		return {
			.left = static_cast<long>(std::floor(position.getX() - getLeft())),
			.top = static_cast<long>(std::floor(position.getY() - getTop())),
			.right = static_cast<long>(std::ceil(position.getX() + getRight())),
			.bottom = static_cast<long>(std::ceil(position.getY() + getBottom()))
		};
	}
};

/**
 * 用于记录具体的实体动态变化
 */
class Velocity {
public:
	struct Period {
		/**
		 * 这一段时间的总移动
		 */
		Vector2D movement;
		/**
		 * 这一段时间的tick长度
		 */
		double tickLasts;
	};

private:
	friend class EntityManager;
	friend class World;
	Vector<Period> periods;

public:
	Velocity() noexcept = default;

	[[nodiscard]] Vector2D getRelativeLocation(double tickDelta) const noexcept {
		Vector2D ret{};
		for (const auto& [movement, tickLasts] : periods)
			if (tickLasts > tickDelta) {
				tickDelta -= tickLasts;
				ret += movement;
			}
			else if (tickLasts != 0) ret += movement * (tickDelta / tickLasts);
		return ret; // 此处估计是tickDelta大于1了，反正无所谓，已经全部加起来了
	}
};

interface IDamageable {
protected:
	double maxHealth = 0;
	double health = 0;

public:
	virtual ~IDamageable() = default;
	virtual void onDamage(Damage&) = 0;
	virtual void onDeath() = 0;
};

interface IArtificialIntelligent {
protected:
	virtual ~IArtificialIntelligent() = default;
	virtual void aiProcess() {}
};

class Entity : public IRenderable, public ITickable {
	friend class Garbage<Entity>;
	friend class World;
	friend class EntityManager;
	EntityID idEntity = 0;
	World* world = nullptr;

protected:
	BoundingBox boundingBox;
	Location location;
	Vector2D velocity;
	Velocity lastVelocity;
	double maxSpeed = 1.0;

	Entity(const Vector2D& location) : location(location) {}
	~Entity() override = default;

	void processVelocity() noexcept;

public:
	virtual void onRemove() { gc.submit<Entity>(this); }
	virtual void onEnterWorld(World* world, WorldTransportReason reason) {}
	virtual void onExitWorld(World* world, WorldTransportReason reason) {}
	virtual void onHit(Entity& /*, Reason*/) {}
	virtual void onGetHit(Entity& /*, Reason*/) {}
	virtual void preDamageDealt(Damage&) {}
	virtual void preDamageTaken(Damage&) {}
	virtual void postDamageDeal(Damage&) {}
	virtual void postDamageTaken(Damage&) {}
	virtual void onApplyDamage(Damage&) {}
	virtual void onBlockDamage(Damage&) {}
	void tick() noexcept override { processVelocity(); }
	void setVelocity(const Vector2D& velocity) noexcept { this->velocity = velocity; }
	[[nodiscard]] const BoundingBox& getBoundingBox() const noexcept { return this->boundingBox; }
	[[nodiscard]] const Location& getLocation() const noexcept { return this->location; }
	[[nodiscard]] const Location& getLocation(const double tickDelta) const noexcept { return Location(this->location.getPosition() + this->velocity * tickDelta, this->location.getWorld()); }
	[[nodiscard]] Vector2D getVelocity() const noexcept { return this->velocity; }
	[[nodiscard]] double getMaxSpeed() const noexcept { return this->maxSpeed; }
	[[nodiscard]] RECT getCoveringBlocks() const noexcept { return boundingBox.getCoveringBlocks(location.getPosition()); }
};

class Enemy : public Entity, public IDamageable, public IArtificialIntelligent {
protected:
	Enemy(const Vector2D& location) : Entity(location) {}
};

class EntityManager {
	friend class Game;
	EntityID nextID = 0;
	Map<EntityID, Entity*> entities; // 所有实体
	EntityManager() = default;
	~EntityManager() { for (auto& [id, entity] : entities) entity->onRemove(); }

public:
	int addEntity(Entity* entity) {
		if (!entity) Failed();
		if (entity->idEntity) Failed();
		entity->idEntity = ++nextID;
		entities.emplace(entity->idEntity, entity);
		Success();
	}

	int removeEntity(Entity* entity) {
		if (!entity) Failed();
		if (entity->idEntity) Failed();
		if (entity->getLocation().getWorld()) Failed(); // 确保必须已经从其他世界移除
		entities.erase(entity->idEntity);
		entity->onRemove();
		Success();
	}
};
