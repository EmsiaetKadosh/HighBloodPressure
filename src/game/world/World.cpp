//
// Created by EmsiaetKadosh on 25-3-30.
//

#pragma once

#include "World.h"
#include "..\Game.h"

int World::adaptEntityVelocity(Entity& entity) {
	if (!entity.world) Failed();
	if (entity.world != this) Failed();
	if (entity.getLocation().getWorld() != idWorld) Failed();
	entity.lastVelocity.periods.clear();
	Vector2D storedVelocity = entity.velocity, velocity = entity.velocity;
	Vector2D position = entity.getLocation().getPosition();
	CollidingSide side = CollidingSide::COVER;
	const double length = velocity.length();
	if (length == 0.0) Failed();
	while (true) {
		const BoundingBoxCollideResults& results = boundingBoxCollideBlocks(entity.boundingBox, entity.location, velocity);
		unsigned int currentOrder = static_cast<unsigned int>(-1);
		for (const BoundingBoxCollideBlockResult& result : results.blocks) {
			Block* block = getBlockAt(result.getBlockLocation());
			if (currentOrder != static_cast<unsigned int>(-1) && result.getOrder() != currentOrder) break; // 进行过了修改，当前order计算完以后就停止计算
			if (!block) continue;
			if (block->adaptEntityVelocity(entity, position, velocity, result.getCollidingSide())) {
				currentOrder = result.getOrder();
				side = result.getCollidingSide();
			}
		}
		entity.lastVelocity.periods.emplace_back(velocity, velocity.length() / length);
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
	Success();
}
