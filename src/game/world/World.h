//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\..\def.h"
#include "..\entity\Entity.h"
#include "..\gameDef.h"
#include "Block.h"

class World : public IRenderable, public ITickable {
	friend class WorldManager;
	friend class Garbage<World>;
	WorldID idWorld = 0;
	Map<QWORD, Entity*> entities;
	Map<BlockLocation, Block*, BlockLocation::Less> blocks;
	AtomicStorage entityGuard;
	AtomicStorage blockGuard;
	using IterEntity = Map<QWORD, Entity*>::const_iterator;
	using IterBlock = Map<BlockLocation, Block*, BlockLocation::Less>::const_iterator;

protected:
	World() = default;
	~World() override = default;

public:
	void tick() noexcept(false) override {
		for (const auto& [_, block] : blocks) block->tick();
		for (const auto& [_, entity] : entities) entity->tick();
	}

	void render(const double tickDelta, const QWORD tickRendering) const noexcept override {
		blockGuard.atomicAcquire(true);
		for (const auto& [location, block] : blocks) block->render(tickDelta, tickRendering);
		blockGuard.atomicRelease(true);
		entityGuard.atomicAcquire(true);
		for (const auto& [id, entity] : entities) entity->render(tickDelta, tickRendering);
		entityGuard.atomicRelease(true);
		for (const auto& [location, block] : blocks) block->renderShadow();
	}

	virtual int addEntity(Entity* entity, const WorldTransportReason reason) noexcept {
		if (!entity) Failed();
		if (!entity->idEntity) Failed();
		if (!reason.isEntityReason()) Failed();
		if (entity->momentum.getLocation().getWorld()) Failed();
		if (entity->world) Failed();
		entity->onEnterWorld(this, reason);
		entity->changeWorld(idWorld);
		entity->world = this;
		entityGuard.atomicAcquire();
		entities.emplace(entity->idEntity, entity);
		entityGuard.atomicRelease();
		Success();
	}

	virtual int removeEntity(Entity* entity, const WorldTransportReason reason) noexcept {
		if (!entity) Failed();
		if (!entity->idEntity) Failed();
		if (!reason.isEntityReason()) Failed();
		entityGuard.atomicAcquire();
		if (!entities.erase(entity->idEntity)) Failed();
		entityGuard.atomicRelease();
		entity->onExitWorld(this, reason);
		entity->changeWorld(idWorld);
		entity->world = nullptr;
		Success();
	}

	virtual int addBlock(Block* block, const WorldTransportReason reason) noexcept {
		if (!block) Failed();
		if (!reason.isBlockReason()) Failed();
		if (block->getLocation().getWorld()) Failed();
		if (block->world) Failed();
		if (blocks.contains(block->getLocation())) Failed();
		blockGuard.atomicAcquire();
		blocks.emplace(block->getLocation(), block);
		blockGuard.atomicRelease();
		block->onEnterWorld(this, reason);
		block->location.setWorld(idWorld);
		block->world = this;
		Success();
	}

	virtual int removeBlock(Block* block, const WorldTransportReason reason) noexcept {
		if (!block) {
			Logger.warn(L"block is nullptr");
			Failed();
		}
		if (!reason.isBlockReason()) {
			Logger.warn(L"reason is not BlockReason: " + reason.toString());
			Failed();
		}
		if (block->getLocation().getWorld() != idWorld) {
			Logger.warn(L"unmatched idWorld: " + std::to_wstring(block->getLocation().getWorld()) + L" but expected " + std::to_wstring(idWorld));
			Failed();
		}
		blockGuard.atomicAcquire();
		if (!blocks.erase(block->getLocation())) {
			Logger.warn(L"no block was removed");
			Failed();
		}
		blockGuard.atomicRelease();
		block->onExitWorld(this, reason);
		block->location.setWorld(0);
		block->world = nullptr;
		Success();
	}

	virtual int removeBlockAt(const BlockLocation& location, const WorldTransportReason reason) noexcept {
		if (!reason.isBlockReason()) Failed();
		if (location.getWorld() != idWorld) Failed();
		const IterBlock it = blocks.find(location);
		if (it == blocks.cend()) Failed();
		Block* block = it->second;
		blockGuard.atomicAcquire();
		blocks.erase(it);
		blockGuard.atomicRelease();
		block->onExitWorld(this, reason);
		block->location.setWorld(0);
		block->world = nullptr;
		Success();
	}

	[[nodiscard]] virtual Block* getBlockAt(const BlockLocation& location) const noexcept {
		const IterBlock it = blocks.find(location);
		if (it == blocks.cend()) return nullptr;
		return it->second;
	}

	virtual void onRemove() noexcept(false);

	/**
	 * @brief 在这个世界中适应一个实体的速度。应当仅在Entity::tick中调用。如果需要别处调用，请注意调用
	 * @code entity->momentum.atomicAcquire() @endcode
	 * @param entity 目标实体
	 * @return int 是否成功
	 */
	void adaptEntityVelocity(Entity& entity) const noexcept(false);

	/**
	 * @brief 获取射线穿过方块的列表。
	 * @param startAt 起始点
	 * @param direction 射线方向、长度
	 */
	[[nodiscard]] RayTraceResults rayTraceBlocks(const Vector2D& startAt, const Vector2D& direction) const noexcept(false);

	/**
	 * @brief 获取一个碰撞箱移动后撞到的方块列表。
	 * @param boundingBox 碰撞箱
	 * @param location 碰撞箱起始位置
	 * @param direction 移动方向
	 */
	[[nodiscard]] BoundingBoxCollideResults boundingBoxCollideBlocks(const BoundingBox& boundingBox, const Location& location, const Vector2D& direction) const noexcept(false);

	/**
	 * @brief 获取实体所受的力场。通常来说是重力场，但考虑到后续一些扩展性，这里需要传入一个实体（包括类型、位置）动态获取力场。
	 * @param entity 探测的实体
	 * @param withGravity 实体是否需要携带重力。考虑有时实体不需要重力判断，或者处于失重状态
	 * @returns 力场，包括重力
	 */
	[[nodiscard]] virtual Vector2D getForceField(Entity& entity, const bool withGravity = true) const noexcept { return { 0, withGravity ? 0.02 : 0 }; }
};

class WorldManager {
	friend class Game;
	WorldID nextID = 0;
	Map<WorldID, World*> worlds;
	World* current = nullptr;
	KeyBinding& speedTweaker = *interactManager.getKeyBindingManager().getRegion(L"world").getBinding(L"speed_tweaker");
	QWORD currentTick = 0;
	using IterWorld = Map<WorldID, World*>::const_iterator;
	WorldManager() = default;

	~WorldManager() {
		Logger.debug(L"~WorldManager() called");
		for (auto& [id, world] : worlds) world->onRemove();
	}

public:
	int addWorld(World* world) {
		if (!world) Failed();
		if (world->idWorld) Failed();
		world->idWorld = ++nextID;
		worlds.emplace(world->idWorld, world);
		Success();
	}

	int removeWorld(World* world) {
		if (!world) Failed();
		if (!world->idWorld) Failed();
		world->idWorld = 0;
		worlds.erase(world->idWorld);
		world->onRemove();
		Success();
	}

	int setWorld(World* world) {
		if (!world) Failed();
		if (!world->idWorld) Failed();
		current = world;
		Success();
	}

	[[nodiscard]] World* getWorld(const WorldID id) const noexcept {
		if (!id) return nullptr;
		const IterWorld world = worlds.find(id);
		if (world == worlds.end()) return nullptr;
		return world->second;
	}

	QWORD getTick() const noexcept { return currentTick; }
	void tick();
};

class StartWorld final : public World {
	StartWorld() = default;
	~StartWorld() override { Logger.debug(L"~StartWorld() called"); }

public:
	static StartWorld* create();
};
