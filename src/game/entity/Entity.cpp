//
// Created by EmsiaetKadosh on 25-3-22.
//

#include "..\Game.h"
#include "Entity.h"
#include "..\world\World.h"

void Entity::updatePosition() noexcept {
	if (momentum.locationTick == game.getTick()) return; // 保护同一tick多次调用
	momentum.atomicAcquire();
	momentum.lastLocation = momentum.location;
	momentum.location.setPosition(momentum.location.getPosition() + momentum.velocity.getRelativeLocation(2));
	momentum.locationTick = game.getTick();
	momentum.velocity.periods.empty() ? velocity.setX(0).setY(0) : velocity = momentum.velocity.periods.back().movement;
	momentum.lastVelocity.periods.clear();
	momentum.velocity.periods.swap(momentum.lastVelocity.periods);
	momentum.atomicRelease();
	checkOnGround();
}

void Entity::checkOnGround() noexcept {
	if (!world) return;
	const long left = static_cast<long>(std::floor(momentum.location.getX() - boundingBox.getLeft()));
	const long right = static_cast<long>(std::ceil(momentum.location.getX() + boundingBox.getRight()));
	const long bottom = static_cast<long>(std::floor(momentum.location.getY() + boundingBox.getBottom()));
	for (long i = left; i < right; ++i)
		for (long j = bottom; j <= bottom + 1; ++j) {
			const BlockLocation bl = BlockLocation(i, j);
			const Block* block = world->getBlockAt(bl);
			if (!block) continue;
			if (block->checkEntityOnGround(*this)) return onGround = true, void();
		}
	onGround = false;
}

void Entity::onDamage(Damage& damage) {
	if (health < damage.getTotalDamage()) health = 0, onDeath();
	else health -= damage.getTotalDamage();
}

void Entity::onDeath() {}

void Entity::tick() noexcept(false) {
	updatePosition();
	if (!world) return;
	if (accelerate.getX() == 0) accelerate.add(isOnGround() ? nRange(-velocity.getX(), -0.2, 0.2) : nRange(-velocity.getX(), -0.03, 0.03), cancelGravityOnce ? 0 : 0.02);
	else if (!cancelGravityOnce) accelerate.add(0, 0.02);
	cancelGravityOnce = false;
	velocity.add(accelerate);
	if (velocity.getY() > 3) velocity.setY(3);
	else if (velocity.getY() < -3) velocity.setY(-3);
	if (velocity.getX() > 3) velocity.setX(3);
	else if (velocity.getX() < -3) velocity.setX(-3);
	world->adaptEntityVelocity(*this);
}

void Entity::render(const double tickDelta, const QWORD tickRendering) const noexcept {
	if (renderer.getCamera().getRenderingTargetEntity() == this) {
		renderer.fillWorld(renderer.getCamera().getTargetPosition().add(boundingBox.getLeftTopOffset()), boundingBox.getWidth(), boundingBox.getHeight(), 0xff44ee66);
		return;
	}
	momentum.atomicAcquire();
	renderer.fillWorld(getLocation(tickDelta, tickRendering).getPosition().add(boundingBox.getLeftTopOffset()), boundingBox.getWidth(), boundingBox.getHeight(), 0xff44ee66);
	momentum.atomicRelease();
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
