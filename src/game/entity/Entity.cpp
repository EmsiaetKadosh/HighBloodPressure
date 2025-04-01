//
// Created by EmsiaetKadosh on 25-3-22.
//

#include "..\Game.h"
#include "Entity.h"
#include "..\world\World.h"

void Entity::updatePosition() noexcept {
	if (momentum.locationTick == game.getTick()) return; // 保护同一tick多次调用
	momentum.atomicAcquire();
	momentum.location.setPosition(momentum.location.getPosition() + momentum.velocity.getRelativeLocation(2));
	momentum.locationTick = game.getTick();
	momentum.velocity.periods.clear();
	momentum.atomicRelease();
}

void Entity::tick() noexcept {
	updatePosition();
	if (!world) return;
	world->adaptEntityVelocity(*this);
}

void Entity::teleport(const Vector2D& location) noexcept {
	momentum.atomicAcquire();
	momentum.location.setPosition(location);
	momentum.locationTick = game.getTick();
	momentum.velocity.periods.clear();
	momentum.atomicRelease();
}

void Entity::changeWorld(const WorldID id, const bool discardMovements) noexcept {
	momentum.atomicAcquire();
	momentum.location.setWorld(id);
	if (discardMovements) {
		momentum.locationTick = game.getTick();
		momentum.velocity.periods.clear();
	}
	momentum.atomicRelease();
}
