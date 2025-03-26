//
// Created by EmsiaetKadosh on 25-3-21.
//

#pragma once

#include "..\..\def.h"
#include "..\entity\Entity.h"
#include "Block.h"

class World;
class WorldManager;

class World : public IRenderable, public ITickable {
	friend class WorldManager;
	friend class Garbage<World>;
	WorldID idWorld = 0;
	Map<QWORD, Entity*> entities;
	Map<BlockLocation, Block*, BlockLocation::Less> blocks;
	using IterEntity = Map<QWORD, Entity*>::const_iterator;
	using IterBlock = Map<BlockLocation, Block*, BlockLocation::Less>::const_iterator;

protected:
	World() = default;
	~World() override = default;

public:
	void render(const double tickDelta) const noexcept override {
		for (const auto& [location, block] : blocks) block->render(tickDelta);
		for (const auto& [id, entity] : entities) entity->render(tickDelta);
		for (const auto& [location, block] : blocks) block->renderShadow();
	}

	virtual int addEntity(Entity* entity, const WorldTransportReason reason) {
		if (!entity) Failed();
		if (!entity->idEntity) Failed();
		if (entity->location.getWorld()) Failed();
		if (entity->world) Failed();
		if (!reason.isEntityReason()) Failed();
		entity->onEnterWorld(this, reason);
		entity->location.setWorld(idWorld);
		entity->world = this;
		entities.emplace(entity->idEntity, entity);
		Success();
	}

	virtual int removeEntity(Entity* entity, const WorldTransportReason reason) {
		if (!entity) Failed();
		if (!entity->idEntity) Failed();
		if (!entities.erase(entity->idEntity)) Failed();
		if (!reason.isEntityReason()) Failed();
		entity->onExitWorld(this, reason);
		entity->location.setWorld(0);
		entity->world = nullptr;
		Success();
	}

	virtual int addBlock(Block* block, const WorldTransportReason reason) {
		if (!block) Failed();
		if (block->getLocation().getWorld()) Failed();
		if (block->world) Failed();
		if (blocks.contains(block->getLocation())) Failed();
		if (!reason.isBlockReason()) Failed();
		blocks.emplace(block->getLocation(), block);
		block->onEnterWorld(this, reason);
		block->location.setWorld(idWorld);
		block->world = this;
		Success();
	}

	virtual int removeBlock(Block* block, const WorldTransportReason reason) {
		if (!block) Failed();
		if (block->getLocation().getWorld() != idWorld) Failed();
		if (!blocks.erase(block->getLocation())) Failed();
		if (!reason.isBlockReason()) Failed();
		block->onExitWorld(this, reason);
		block->location.setWorld(0);
		block->world = nullptr;
		Success();
	}

	virtual int removeBlockAt(const BlockLocation& location, const WorldTransportReason reason) {
		if (!reason.isBlockReason()) Failed();
		if (location.getWorld() != idWorld) Failed();
		const IterBlock it = blocks.find(location);
		if (it == blocks.cend()) Failed();
		Block* block = it->second;
		blocks.erase(it);
		block->onExitWorld(this, reason);
		block->location.setWorld(0);
		block->world = nullptr;
		Success();
	}

	virtual void onRemove() {
		// Entity不需要再此处删除，交给EntityManager管理
		for (auto& [id, entity] : entities) {
			entity->onExitWorld(this, WorldTransportReason::WorldCollapse);
			entity->location.setWorld(0);
			entity->world = nullptr;
		}
		for (auto& [location, block] : blocks) {
			block->onExitWorld(this, WorldTransportReason::WorldCollapse);
			block->location.setWorld(0);
			block->world = nullptr;
			block->onRemove();
		}
		gc.submit<World>(this);
		entities.clear();
		blocks.clear();
		Logger.debug(L"World::onRemove() called");
	}
};

class WorldManager {
	friend class Game;
	WorldID nextID = 0;
	Map<WorldID, World*> worlds;
	World* current = nullptr;
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
};

class StartWorld final : public World {
	StartWorld() = default;
	~StartWorld() override { Logger.debug(L"~StartWorld() called"); }

public:
	void tick() noexcept override {}

	static StartWorld* create() {
		StartWorld* world = allocatedFor(new StartWorld);
		Block* block;
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(-2, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(-1, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(0, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(1, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(2, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		return world;
	}
};
