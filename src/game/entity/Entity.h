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
		if constexpr (false) {
			RECT ret;
			const double
				left = position.getX() - getLeft(),
				top = position.getY() - getTop(),
				right = position.getX() + getRight(),
				bottom = position.getY() + getBottom();
			double temp;
			if (dEquals(left, temp = std::ceil(left))) ret.left = static_cast<long>(temp);
			else ret.left = static_cast<long>(std::floor(left));
			if (dEquals(right, temp = std::floor(right))) ret.right = static_cast<long>(temp);
			else ret.right = static_cast<long>(std::ceil(right));
			if (dEquals(top, temp = std::ceil(top))) ret.top = static_cast<long>(temp);
			else ret.top = static_cast<long>(std::floor(top));
			if (dEquals(bottom, temp = std::floor(bottom))) ret.bottom = static_cast<long>(temp);
			else ret.bottom = static_cast<long>(std::ceil(bottom));
			return ret;
		}
		return {
			.left = static_cast<long>(std::floor(position.getX() - getLeft())),
			.top = static_cast<long>(std::floor(position.getY() - getTop())),
			.right = static_cast<long>(std::ceil(position.getX() + getRight())),
			.bottom = static_cast<long>(std::ceil(position.getY() + getBottom()))
		};
	}

	[[nodiscard]] String toString(const Vector2D& position) const {
		return L"left = " + dtoString(position.getX() - left) + L", right = " + dtoString(position.getX() + right) + L", top = " + dtoString(position.getY() - top) + L", bottom = " + dtoString(position.getY() + bottom);
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
	friend class Garbage<Velocity>;
	friend class EntityManager;
	friend class Entity;
	friend class World;
	Vector<Period> periods;

public:
	Velocity() noexcept = default;
	~Velocity() noexcept = default;

	[[nodiscard]] Vector2D getRelativeLocation(double tickDelta) const noexcept {
		Vector2D ret;
		for (const auto& [movement, tickLasts] : periods)
			if (tickDelta > tickLasts) {
				tickDelta -= tickLasts;
				ret += movement;
			}
			else if (tickLasts != 0) ret += movement * nRange(tickDelta / tickLasts, 0.0, 1.0);
		return ret; // 此处估计是tickDelta大于1了，反正无所谓，已经全部加起来了
	}

	[[nodiscard]] String toString() const noexcept {
		std::wostringstream stream;
		for (const auto& [movement, tickLasts] : periods) stream << L"\n    movement: " << movement.toString() << L"\n    tickLasts: " << tickLasts << L"\n  --------";
		return stream.str();
	}
};

class EntityMomentum final : public AtomicStorage {
	friend class Entity;
	friend class World;
	Location lastLocation;
	Velocity lastVelocity;
	Location location;
	Velocity velocity;
	QWORD locationTick = 0;
	QWORD velocityTick = 0;

public:
	EntityMomentum(const Location& location, const Velocity& velocity) noexcept : lastLocation(location), lastVelocity(velocity), location(location), velocity(velocity) {}
	~EntityMomentum() noexcept override = default;
	[[nodiscard]] const Location& getLocation() const noexcept { return location; }
	[[nodiscard]] const Velocity& getVelocity() const noexcept { return velocity; }
	[[nodiscard]] QWORD getLocationTick() const noexcept { return locationTick; }
	[[nodiscard]] QWORD getVelocityTick() const noexcept { return velocityTick; }
};

struct IArtificialIntelligent {
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
	BoundingBox boundingBox = BoundingBox();
	EntityMomentum momentum;
	Vector2D velocity = Vector2D();
	Vector2D accelerate = Vector2D();
	double maxSpeed = 1.0;
	double maxHealth = 100;
	double health = 100;
	double maxBloodPressure = 100;
	double bloodPressure = 0;
	bool onGround = true;
	bool cancelGravityOnce = false;

	Entity(const Vector2D& location) : momentum(location, Velocity()) {}
	~Entity() override = default;

	virtual void updatePosition() noexcept;

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
	virtual void checkOnGround() noexcept;
	virtual void onDamage(Damage& damage);
	virtual void onDeath();
	void tick() noexcept(false) override;
	void render(double tickDelta, QWORD tickRendering) const noexcept override;
	void setVelocity(const Vector2D& velocity) noexcept { this->velocity = velocity; }
	void teleport(const Vector2D& location) noexcept;
	void changeWorld(WorldID id, bool discardMovements = true) noexcept;
	void setOnGround(const bool val) noexcept { onGround = val; }
	void cancelGravityThisTick() noexcept { cancelGravityOnce = true; }
	[[nodiscard]] const BoundingBox& getBoundingBox() const noexcept { return this->boundingBox; }
	[[nodiscard]] Location getLocation() const noexcept { return momentum.location; }
	[[nodiscard]] Vector2D getVelocity() const noexcept { return this->velocity; }
	[[nodiscard]] Vector2D getAcceleration() const noexcept { return this->accelerate; }
	[[nodiscard]] double getMaxSpeed() const noexcept { return this->maxSpeed; }
	[[nodiscard]] RECT getCoveringBlocks() const noexcept { return boundingBox.getCoveringBlocks(momentum.location.getPosition()); }
	[[nodiscard]] EntityMomentum& getMomentum() noexcept { return this->momentum; }
	[[nodiscard]] bool isOnGround() const noexcept { return onGround; }

	/**
	 * @brief 根据tickDelta和tickRendering获取Location。因为一些原因，请在外部记得
	 * @code this->momentum.atomicAcquire() @endcode
	 */
	[[nodiscard]] Location getLocation(const double tickDelta, const QWORD tickRendering) const noexcept {
		// const AtomicGuard guard = momentum.atomicGuard();
		// assert momentum.locationTick <= tickRendering && momentum.velocityTick <= tickRendering;
		if (momentum.locationTick > tickRendering) return momentum.lastLocation.getPosition() + momentum.lastVelocity.getRelativeLocation(tickDelta);
		return momentum.location.getPosition() + momentum.velocity.getRelativeLocation(tickDelta);
	}
};

class Enemy : public Entity, public IArtificialIntelligent {
protected:
	Enemy(const Vector2D& location) : Entity(location) {}
};

class EntityManager {
	friend class Game;
	EntityID nextID = 0;
	Map<EntityID, Entity*> entities; // 所有实体
	using IterEntity = Map<EntityID, Entity*>::const_iterator;
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

	[[nodiscard]] Entity* getEntity(const EntityID id) const noexcept {
		const IterEntity iter = entities.find(id);
		if (iter == entities.end()) return nullptr;
		return iter->second;
	}
};

class EntityList;

class EntityEntry final : public AnywhereEditable<EntityEntry, EntityList> {
public:
	Entity* const entity;
	EntityEntry(Entity* const entity) : entity(entity) {}
	Entity* operator->() const noexcept { return this->entity; }
	Entity& operator*() const noexcept { return *this->entity; }
};

class EntityList final : public AnywhereEditableList<EntityEntry, EntityList> {

};


