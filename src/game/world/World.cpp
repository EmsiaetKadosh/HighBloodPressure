//
// Created by EmsiaetKadosh on 25-3-30.
//

#pragma once

#include "World.h"
#include "..\Game.h"

void World::adaptEntityVelocity(Entity& entity) const {
	if (!entity.world) return entity.momentum.velocityTick = game.getTick(), void();
	if (entity.world != this) return entity.momentum.velocityTick = game.getTick(), void();
	if (entity.getLocation().getWorld() != idWorld) return entity.momentum.velocityTick = game.getTick(), void();
	entity.momentum.velocity.periods.clear(); // 此函数在此处不需要atomicAcquire
	Vector2D storedVelocity = entity.velocity, velocity = entity.velocity;
	Vector2D position = entity.getLocation().getPosition();
	CollidingSide side = CollidingSide::COVER;
	double length = 0.0;
	double timeLeft = 1.0;
	while (true) {
		length = velocity.length();
		if (length == 0.0) break;
		const auto& [blocks, _] = boundingBoxCollideBlocks(entity.boundingBox, entity.momentum.location /* 此处在Entity::tick中确保是最新的 */, velocity);
		unsigned int currentOrder = static_cast<unsigned int>(-1);
		for (const BoundingBoxCollideBlockResult& result : blocks) {
			const Block* block = getBlockAt(result.getBlockLocation());
			if (currentOrder != static_cast<unsigned int>(-1) && result.getOrder() != currentOrder) break; // 进行过了修改，当前order计算完以后就停止计算
			if (!block) continue;
			if (block->adaptEntityVelocity(entity, position, velocity, result.getCollidingSide())) {
				currentOrder = result.getOrder();
				side = result.getCollidingSide();
			}
		}
		double timeCost = velocity.length() * timeLeft / length;
		if (timeCost > timeLeft) {
			Logger.warn(L"timeCost > timeLeft !!! timeCost = " + dtoString(timeCost) + L", timeLeft = " + dtoString(timeLeft));
			timeCost = timeLeft;
		}
		entity.momentum.velocity.periods.emplace_back(velocity, timeCost);
		timeLeft -= timeCost;
		position.add(velocity);
		storedVelocity.subtract(velocity);
		switch (side) {
			case CollidingSide::LEFT:
			case CollidingSide::RIGHT:
				storedVelocity.setX(0);
				break;
			case CollidingSide::TOP:
			case CollidingSide::BOTTOM:
				storedVelocity.setY(0);
				break;
			case CollidingSide::LEFT_TOP:
			case CollidingSide::RIGHT_TOP:
			case CollidingSide::RIGHT_BOTTOM:
			case CollidingSide::LEFT_BOTTOM:
				if (std::abs(storedVelocity.getX()) > std::abs(storedVelocity.getY())) storedVelocity.setY(0);
				else if (std::abs(storedVelocity.getX()) < std::abs(storedVelocity.getY())) storedVelocity.setX(0);
				else if (game.random() % 2) storedVelocity.setY(0);
				else storedVelocity.setX(0);
				break;
			case CollidingSide::COVER:
				break;
			default:
				unreachable();
		}
		if (storedVelocity.lengthManhattan() == 0) break;
		velocity = storedVelocity;
	}
	return entity.momentum.velocityTick = game.getTick(), void();
}

RayTraceResults World::rayTraceBlocks(const Vector2D& startAt, const Vector2D& direction) const {
	RayTraceResults results;
	const Vector2D& endPoint = startAt + direction;
	long xMin, yMin, xMax, yMax;
	if (startAt.getX() < endPoint.getX()) {
		xMin = static_cast<long>(std::floor(startAt.getX()));
		xMax = static_cast<long>(std::ceil(endPoint.getX()));
	} else if (startAt.getX() > endPoint.getX()) {
		xMin = static_cast<long>(std::floor(endPoint.getX()));
		xMax = static_cast<long>(std::ceil(startAt.getX()));
	} else { // x == x, 特殊处理
		xMin = static_cast<long>(std::floor(startAt.getX()));
		if (direction.getY() == 0) { // No movement
			results.locations.emplace_back(BlockLocation(xMin, static_cast<long>(std::floor(startAt.getY())), idWorld), startAt, CollidingSide::COVER);
			return results;
		}
		if (direction.getY() > 0) { // 向下移动
			yMin = static_cast<long>(std::ceil(startAt.getY())), yMax = static_cast<long>(std::ceil(endPoint.getY()));
			if (yMin < yMax) {
				long y = yMin;
				if (BlockLocation::blockCompletelyContains(Vector2D(xMin, y), startAt)) results.locations.emplace_back(BlockLocation(xMin, y, idWorld), startAt, CollidingSide::COVER);
				else results.locations.emplace_back(BlockLocation(xMin, y, idWorld), Vector2D(startAt.getX(), y), CollidingSide::TOP);
				for (++y; y < yMax; ++y) results.locations.emplace_back(BlockLocation(xMin, y, idWorld), Vector2D(startAt.getX(), y), CollidingSide::TOP);
			}
		} else { // 向上移动
			yMin = static_cast<long>(std::floor(endPoint.getY())), yMax = static_cast<long>(std::floor(startAt.getY()));
			if (yMin < yMax) {
				long y = yMax - 1;
				if (BlockLocation::blockCompletelyContains(Vector2D(xMin, y), startAt)) results.locations.emplace_back(BlockLocation(xMin, y, idWorld), startAt, CollidingSide::COVER);
				else results.locations.emplace_back(BlockLocation(xMin, y, idWorld), Vector2D(startAt.getX(), y + 1), CollidingSide::BOTTOM);
				for (--y; y >= yMin; --y) results.locations.emplace_back(BlockLocation(xMin, y, idWorld), Vector2D(startAt.getX(), y + 1), CollidingSide::BOTTOM);
			}
		}
		return results;
	}
	if (direction.getY() > 0) {
		yMin = static_cast<long>(std::floor(startAt.getY()));
		yMax = static_cast<long>(std::ceil(endPoint.getY()));
	} else if (direction.getY() < 0) {
		yMin = static_cast<long>(std::floor(endPoint.getY()));
		yMax = static_cast<long>(std::ceil(startAt.getY()));
	} else { // y == y, 特殊处理
		yMin = static_cast<long>(std::floor(startAt.getY()));
		if (direction.getX() > 0) { // 向右
			if (xMin < xMax) {
				long x = xMin;
				if (BlockLocation::blockCompletelyContains(Vector2D(x, yMin), startAt)) results.locations.emplace_back(BlockLocation(x, yMin, idWorld), startAt, CollidingSide::COVER);
				else results.locations.emplace_back(BlockLocation(x, yMin, idWorld), Vector2D(x, startAt.getY()), CollidingSide::LEFT);
				for (++x; x < xMax; ++x) results.locations.emplace_back(BlockLocation(x, yMin, idWorld), Vector2D(x, startAt.getY()), CollidingSide::LEFT);
			}
		} else { //
			// assert x != 0 && x > 0;
			if (xMin < xMax) {
				long x = xMax - 1;
				if (BlockLocation::blockCompletelyContains(Vector2D(x, yMin), startAt)) results.locations.emplace_back(BlockLocation(x, yMin, idWorld), startAt, CollidingSide::COVER);
				else results.locations.emplace_back(BlockLocation(x, yMin, idWorld), Vector2D(x + 1, startAt.getY()), CollidingSide::RIGHT);
				for (--x; x >= xMin; --x) results.locations.emplace_back(BlockLocation(x, yMin, idWorld), Vector2D(x + 1, startAt.getY()), CollidingSide::RIGHT);
			}
		}
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
				pass;
				// throw ZeroValueException(L"Vector2D::extendValue X/Y returned zero Vector2D:");
			}
			ex.add(startAt);
			if (fix.getX() == 0) ex.setY(std::round(ex.getY()));
			else ex.setX(std::round(ex.getX()));
			results.locations.emplace_back(BlockLocation(x, y, idWorld), ex, CollidingSide::fromVector2D(fix));
		}
	return results;
}

BoundingBoxCollideResults World::boundingBoxCollideBlocks(const BoundingBox& boundingBox, const Location& location, const Vector2D& direction) const {
	BoundingBoxCollideResults results = BoundingBoxCollideResults();
	const Vector2D position = location.getPosition();
	const auto [coverLeft, coverTop, coverRight, coverBottom] = boundingBox.getCoveringBlocks(position);
	const auto [reflectLeft, reflectTop, reflectRight, reflectBottom] = boundingBox.getCoveringBlocks(position + direction);
	const RECT range = {
		.left = nMin(coverLeft, reflectLeft),
		.top = nMin(coverTop, reflectTop),
		.right = nMax(coverRight, reflectRight),
		.bottom = nMax(coverBottom, reflectBottom)
	};
	if (direction.getX() == 0) { // 纵向运动
		if (direction.getY() == 0) for (long x = range.left; x < range.right; ++x) for (long y = range.top; y < range.bottom; ++y) results.blocks.emplace(BlockLocation(x, y, idWorld), 0, CollidingSide::COVER);
		else if (direction.getY() < 0) // 向上
			for (long y = range.bottom - 1; y >= range.top; --y)
				for (long x = range.left; x < range.right; ++x) {
					if (nSideBetween(x, coverLeft, coverRight) && nSideBetween(y, coverTop, coverBottom)) continue;
					results.blocks.emplace(BlockLocation(x, y, idWorld), range.bottom - y, CollidingSide(CollidingSide::BOTTOM));
				}
		else // 向下
			for (long y = range.top; y < range.bottom; ++y)
				for (long x = range.left; x < range.right; ++x) {
					if (nSideBetween(x, coverLeft, coverRight) && nSideBetween(y, coverTop, coverBottom)) continue;
					results.blocks.emplace(BlockLocation(x, y, idWorld), y - range.top, CollidingSide::TOP);
				}
		return results;
		// 对x == 0的特殊处理没有顺序问题
	}
	if (direction.getY() == 0) { // 横向运动
		if (direction.getX() > 0) // 向右
			for (long x = range.left; x < range.right; ++x)
				for (long y = range.top; y < range.bottom; ++y) {
					if (nSideBetween(x, coverLeft, coverRight) && nSideBetween(y, coverTop, coverBottom)) continue;
					results.blocks.emplace(BlockLocation(x, y, idWorld), x - range.left, CollidingSide::LEFT);
				}
		else // 向左
			for (long x = range.right - 1; x >= range.left; --x)
				for (long y = range.top; y < range.bottom; ++y) {
					if (nSideBetween(x, coverLeft, coverRight) && nSideBetween(y, coverTop, coverBottom)) continue;
					results.blocks.emplace(BlockLocation(x, y, idWorld), range.right - x, CollidingSide::RIGHT);
				}
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
	const auto& [locations, _] = rayTraceBlocks(coverForward, direction.clone().multiply(1.08)); // 此处需要获得定序方式
	unsigned int od = 0;
	for (const auto& block : locations)
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
			if (nSideBetween(x, coverLeft, coverRight) && nSideBetween(y, coverTop, coverBottom)) continue; // Cover的直接扔了得了
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
