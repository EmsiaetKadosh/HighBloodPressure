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
	using IterEntity = Map<QWORD, Entity*>::const_iterator;
	using IterBlock = Map<BlockLocation, Block*, BlockLocation::Less>::const_iterator;

protected:
	World() = default;
	~World() override = default;

public:
	void tick() noexcept override {
		for (const auto& [_, block] : blocks) block->tick();
		for (const auto& [_, entity] : entities) entity->tick();
	}

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

	[[nodiscard]] virtual Block* getBlockAt(const BlockLocation& location) const {
		const IterBlock it = blocks.find(location);
		if (it == blocks.cend()) return nullptr;
		return it->second;
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

	int adaptEntityVelocity(Entity& entity);

	[[nodiscard]] RayTraceResults rayTraceBlocks(const Vector2D& startAt, const Vector2D& direction) const {
		RayTraceResults results;
		const Vector2D& endPoint = startAt + direction;
		long xMin, yMin, xMax, yMax;
		if (startAt.getX() < endPoint.getX()) {
			xMin = static_cast<long>(std::floor(startAt.getX()));
			xMax = static_cast<long>(std::ceil(endPoint.getX()));
		}
		else if (startAt.getX() > endPoint.getX()) {
			xMin = static_cast<long>(std::floor(endPoint.getX()));
			xMax = static_cast<long>(std::ceil(startAt.getX()));
		}
		else { // x == x, 特殊处理
			xMin = static_cast<long>(std::floor(startAt.getX()));
			results.locations.emplace_back(BlockLocation(xMin, static_cast<long>(std::floor(startAt.getY())), idWorld), startAt, CollidingSide::COVER);
			if (startAt.getY() == endPoint.getY()) return results;
			if (startAt.getY() < endPoint.getY()) {
				yMin = static_cast<long>(std::ceil(startAt.getY())), yMax = static_cast<long>(std::ceil(endPoint.getY()));
				for (long y = yMin; y < yMax; ++y) results.locations.emplace_back(BlockLocation(xMin, y, idWorld), Vector2D(startAt.getX(), y), CollidingSide::LEFT);
			}
			else {
				yMin = static_cast<long>(std::floor(endPoint.getY())), yMax = static_cast<long>(std::floor(startAt.getY()));
				for (long y = yMax - 1; y >= yMin; --y) results.locations.emplace_back(BlockLocation(xMin, y, idWorld), Vector2D(startAt.getX(), y), CollidingSide::RIGHT);
			}
			return results;
		}
		if (startAt.getY() < endPoint.getY()) {
			yMin = static_cast<long>(std::floor(startAt.getY()));
			yMax = static_cast<long>(std::ceil(endPoint.getY()));
		}
		else if (startAt.getY() > endPoint.getY()) {
			yMin = static_cast<long>(std::floor(endPoint.getY()));
			yMax = static_cast<long>(std::ceil(startAt.getY()));
		}
		else { // y == y, 特殊处理
			yMin = static_cast<long>(std::floor(startAt.getY()));
			if (xMin < xMax) for (long x = xMin; x < xMax; ++x) results.locations.emplace_back(BlockLocation(x, yMin, idWorld), Vector2D(x, startAt.getY()), CollidingSide::LEFT);
			else for (long x = xMax - 1; x >= xMin; --x) results.locations.emplace_back(BlockLocation(x, yMin, idWorld), Vector2D(x, startAt.getY()), CollidingSide::RIGHT);
			return results;
		}
		const Vector2D& fourWay = direction.getDiagonalFourWay().multiply(-0.5); // 这里是没有四向顺时针约化的问题的
		const long xDelta = fourWay.getX() < 0 ? 1 : -1;
		const long yDelta = fourWay.getY() < 0 ? 1 : -1;
		const double amMax = 0.5 * direction.lengthManhattan();
		for (long x = xDelta > 0 ? xMin : xMax - 1; xDelta > 0 ? x < xMax : x >= xMin; x += xDelta)
			for (long y = yDelta > 0 ? yMin : yMax - 1; yDelta > 0 ? y < yMax : y >= yMin; y += yDelta) {
				const Vector2D block = Vector2D(x + 0.5, y + 0.5); // 实际方块中心
				Vector2D&& amRelativeP1 = block - startAt; // 中心相对位置
				if (const double amP1 = std::abs(direction.getX() * amRelativeP1.getY() - direction.getY() * amRelativeP1.getX()); amP1 > amMax) continue; // 相交
				const double cross = direction.cross(amRelativeP1 + fourWay /* 减去四向，得到最近点与起始点的差 */).getZ();
				if (cross == 0) { // 零叉乘，也就是正好撞角
					results.locations.emplace_back(BlockLocation(x, y, idWorld), block + fourWay, CollidingSide::fromVector2D(fourWay));
					continue;
				}
				// 正叉乘：顺时针转一下（指的是，方块中心到撞击边/角的偏移）
				// 负叉乘：逆时针转一下（指的是，方块中心到撞击边/角的偏移）
				const Vector2D fix = (cross > 0 ? CollidingSide::fromVector2D(fourWay).getClockwiseRotated() : CollidingSide::fromVector2D(fourWay).getAntiClockwiseRotated()).getDirectionBlock().multiply(0.5);
				Vector2D ex = fix.getX() == 0 ? direction.clone().extendValueY(block.getY() + fix.getY() - startAt.getY()) : direction.clone().extendValueX(block.getX() + fix.getX() - startAt.getX());
				if (ex.isZero()) {
					Logger.warn(
						L"Vector2D::extendValue X/Y returned zero Vector2D:"
						L"\n    startAt: " + startAt.toString() +
						L"\n    direction: " + direction.toString() +
						L"\n    blockCenter: " + block.toString() +
						L"\n    antimatterRelativeP1: " + amRelativeP1.toString() +
						L"\n    cross: " + std::to_wstring(cross) +
						L"\n    fix: " + fix.toString() +
						L"\n    fourWay: " + fourWay.toString() +
						L"\n    ex: " + ex.toString()
					);

					// throw ZeroValueException(L"Vector2D::extendValue X/Y returned zero Vector2D:");
				}
				ex.add(startAt);
				if (fix.getX() == 0) ex.setY(std::round(ex.getY()));
				else ex.setX(std::round(ex.getX()));
				results.locations.emplace_back(BlockLocation(x, y, idWorld), ex, CollidingSide::fromVector2D(fix));
			}
		return results;
	}

	[[nodiscard]] BoundingBoxCollideResults boundingBoxCollideBlocks(const BoundingBox& boundingBox, const Location& location, const Vector2D& direction) const {
		BoundingBoxCollideResults results = BoundingBoxCollideResults();
		const Vector2D position = location.getPosition();
		const RECT cover = boundingBox.getCoveringBlocks(position);
		const RECT reflect = boundingBox.getCoveringBlocks(position + direction);
		const RECT range = {
			.left = nMin(cover.left, reflect.left),
			.top = nMin(cover.top, reflect.top),
			.right = nMax(cover.right, reflect.right),
			.bottom = nMax(cover.bottom, reflect.bottom)
		};
		if (direction.getX() == 0) {
			if (direction.getY() == 0) {
				for (long x = range.left; x < range.right; ++x) for (long y = range.top; y < range.bottom; ++y) results.blocks.emplace(BlockLocation(x, y, idWorld), 0, CollidingSide::COVER);
				return results;
			}
			if (direction.getY() < 0) for (long y = range.bottom - 1; y >= range.top; --y) for (long x = range.left; x < range.right; ++x) results.blocks.emplace(BlockLocation(x, y, idWorld), range.bottom - y, CollidingSide(CollidingSide::TOP));
			else for (long y = range.top; y < range.bottom; ++y) for (long x = range.left; x < range.right; ++x) results.blocks.emplace(BlockLocation(x, y, idWorld), y - range.top, CollidingSide::BOTTOM);
			return results;
			// 对x == 0的特殊处理没有顺序问题
		}
		if (direction.getY() == 0) {
			if (direction.getX() > 0) for (long x = range.left; x < range.right; ++x) for (long y = range.top; y < range.bottom; ++y) results.blocks.emplace(BlockLocation(x, y, idWorld), x - range.left, CollidingSide::LEFT);
			else for (long x = range.right - 1; x >= range.left; --x) for (long y = range.top; y < range.bottom; ++y) results.blocks.emplace(BlockLocation(x, y, idWorld), range.right - x, CollidingSide::RIGHT);
			return results;
			// 对y == 0的特殊处理没有顺序问题
		}
		const Vector2D fourWay = direction.getDiagonalFourWay().multiply(-0.5);
		const CollidingSide nearestSide = CollidingSide::fromVector2D(fourWay);
		Vector2D coverFarthestPositive, coverFarthestNegative;
		Vector2D coverForward, reflectForward; // 此处reflectForward在getForefrontOffset中无效，用于占位，后作他用
		boundingBox.getFarthestOffset(coverFarthestPositive, coverFarthestNegative, direction);
		boundingBox.getForefrontOffset(coverForward, reflectForward, direction);
		reflectForward = coverForward + direction;
		coverFarthestPositive.add(position);
		coverFarthestNegative.add(position);
		coverForward.add(position);
		reflectForward.add(position);
		Set<$LimitedAccess::BoundingBoxTraceOrder, $LimitedAccess::BoundingBoxTraceLessX> xOrder;
		Set<$LimitedAccess::BoundingBoxTraceOrder, $LimitedAccess::BoundingBoxTraceLessY> yOrder;
		const RayTraceResults& rayTrace = rayTraceBlocks(coverForward, direction); // 此处需要获得定序方式
		unsigned int od = 0;
		for (const auto& block : rayTrace.locations)
			switch (block.getHitSide()) {
				case CollidingSide::LEFT:
				case CollidingSide::RIGHT:
					xOrder.emplace(std::round(block.getHitPoint().getX()), ++od);
					break;
				case CollidingSide::TOP:
				case CollidingSide::BOTTOM:
					yOrder.emplace(std::round(block.getHitPoint().getY()), ++od);
					break;
				case CollidingSide::LEFT_TOP:
				case CollidingSide::RIGHT_TOP:
				case CollidingSide::LEFT_BOTTOM:
				case CollidingSide::RIGHT_BOTTOM:
					xOrder.emplace(std::round(block.getHitPoint().getX()), ++od);
					yOrder.emplace(std::round(block.getHitPoint().getY()), od);
					break;
				case CollidingSide::COVER:
					break;
				default:
					unreachable();
			} // 完成定序
		const double amMax = 0.5 * direction.lengthManhattan();
		const double amH = std::abs(direction.getX() * (coverFarthestPositive.getY() - coverFarthestNegative.getY()) - direction.getY() * (coverFarthestPositive.getX() - coverFarthestNegative.getX()));
		for (long x = range.left; x < range.right; ++x)
			for (long y = range.top; y < range.bottom; ++y) {
				if (cover.left <= x && x < cover.right && cover.top <= y && y < cover.bottom) continue; // Cover的直接扔了得了
				const Vector2D blockCenter = Vector2D(x + 0.5, y + 0.5);
				const Vector2D& amRelativeP1 = blockCenter - coverFarthestPositive;
				const Vector2D& amRelativeP2 = blockCenter - coverFarthestNegative;
				const double amP1 = std::abs(direction.getX() * amRelativeP1.getY() - direction.getY() * amRelativeP1.getX());
				const double amP2 = std::abs(direction.getX() * amRelativeP2.getY() - direction.getY() * amRelativeP2.getX());
				if (amP1 < amMax || amP2 < amMax) goto append; // 相交
				if (dEquals(amP1 + amP2, amH)) goto append; // 在内
				if (Vector2D&& farthestRelative = blockCenter - reflectForward; !nSamePositivity(farthestRelative.getX(), direction.getX()) && !nSamePositivity(farthestRelative.getY(), direction.getY())) goto append;
				continue;
			append:
				CollidingSide side; // assert !CollidingSide::COVER;
				if (const double e = (blockCenter + fourWay - coverForward /* 最近点与前向点的坐标差 */).cross(direction).getZ(); e == 0) side = nearestSide;
				else if (e > 0) side = nearestSide.getAntiClockwiseRotated();
				else side = nearestSide.getClockwiseRotated();
				BoundingBoxCollideBlockResult result = BoundingBoxCollideBlockResult(BlockLocation(blockCenter, idWorld), 0, side);
				if (side == CollidingSide::TOP || side == CollidingSide::BOTTOM) result.order = yOrder.find(result)->order;
				else result.order = xOrder.find(result)->order;
				results.blocks.emplace(result);
			}
		return results;
	}
};

class WorldManager {
	friend class Game;
	WorldID nextID = 0;
	Map<WorldID, World*> worlds;
	World* current = nullptr;
	WorldManager() = default;
	using IterWorld = Map<WorldID, World*>::const_iterator;

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

	World* getWorld(const WorldID id) const noexcept {
		if (!id) return nullptr;
		const IterWorld world = worlds.find(id);
		if (world == worlds.end()) return nullptr;
		return world->second;
	}
};

class StartWorld final : public World {
	StartWorld() = default;
	~StartWorld() override { Logger.debug(L"~StartWorld() called"); }

public:
	static StartWorld* create() {
		StartWorld* world = allocatedFor(new StartWorld);
		PureBarrierBlock* block;
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(-2, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(-1, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(0, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		else block->setColor(0xff4488ee);
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(1, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		if (world->addBlock(block = PureBarrierBlock::create(BlockLocation(2, 0)), WorldTransportReason::InitialGeneration)) block->onRemove();
		return world;
	}
};
