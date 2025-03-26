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

};

interface IDamageable {
protected:
	double maxHealth = 0;
	double health = 0;

public:
	virtual ~IDamageable() = default;
	virtual void Damage(Damage&) = 0;
	virtual void onDeath() = 0;
};

interface IArtificialIntelligent {
protected:
	virtual ~IArtificialIntelligent() = default;
	virtual void aiProcess() {}
};

class Entity : public IRenderable, public ITickable {
	friend class World;
	friend class EntityManager;
	EntityID idEntity = 0;
	World* world = nullptr;

protected:
	Location location;
	Vector2D velocity;
	Vector2D lastVelocity;
	double maxSpeed = 1.0;

	Entity(const Vector2D& location) : location(location) {}
	~Entity() override = default;

	void processVelocity() noexcept;

public:
	virtual void onRemove() = 0;
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

	void setVelocity(const Vector2D& velocity) noexcept { this->velocity = velocity; }
	[[nodiscard]] const Location& getLocation() const noexcept { return this->location; }
	[[nodiscard]] const Location& getLocation(const double tickDelta) const noexcept { return Location(this->location.getPosition() + this->velocity * tickDelta, this->location.getWorld()); }
	[[nodiscard]] Vector2D getVelocity() const noexcept { return this->velocity; }
	[[nodiscard]] double getMaxSpeed() const noexcept { return this->maxSpeed; }
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
	~EntityManager() { for (auto& [id, entity] : entities) { entity->onRemove(); } }

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
