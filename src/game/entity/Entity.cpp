//
// Created by EmsiaetKadosh on 25-3-22.
//

#include "..\Game.h"
#include "Entity.h"
#include "..\world\World.h"

void BoundingBox::getFarthestOffset(Vector2D& outPointPositive, Vector2D& outPointNegative, const Vector2D& direction) const noexcept {
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

void BoundingBox::getForefrontOffset(Vector2D& outPointForward, Vector2D& outPointBackward, const Vector2D& direction) const noexcept {
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

RECT BoundingBox::getCoveringBlocks(const Vector2D& position) const noexcept {
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

Vector2D Velocity::getRelativeLocation(double tickDelta) const noexcept {
	Vector2D ret;
	for (const auto& [movement, tickLasts] : periods)
		if (tickDelta > tickLasts) {
			tickDelta -= tickLasts;
			ret += movement;
		} else if (tickLasts != 0) ret += movement * nRange(tickDelta / tickLasts, 0.0, 1.0);
	return ret; // 此处估计是tickDelta大于1了，反正无所谓，已经全部加起来了
}

String Velocity::toString() const noexcept {
	std::wostringstream stream;
	for (const auto& [movement, tickLasts] : periods) stream << L"\n    movement: " << movement.toString() << L"\n    tickLasts: " << tickLasts << L"\n  --------";
	return stream.str();
}

void Entity::updatePosition() noexcept {
	if (momentum.locationTick == game.getWorldManager().getTick()) return; // 保护同一tick多次调用
	momentum.atomicAcquire();
	momentum.lastLocation = momentum.location;
	momentum.location.setPosition(momentum.location.getPosition() + momentum.velocity.getRelativeLocation(2));
	momentum.locationTick = game.getWorldManager().getTick();
	momentum.velocity.periods.empty() ? velocity.setX(0).setY(0) : velocity = momentum.velocity.getLastVelocity();
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

void Entity::setHealth(const double health) noexcept {
	if (health < 0) this->health = 0;
	else if (health > maxHealth) this->health = maxHealth;
	else this->health = health;
}

void Entity::setMaxHealth(const double health) noexcept {
	if (health < 0) maxHealth = 0;
	else maxHealth = health;
}

void Entity::setBloodPressure(const double pressure) noexcept {
	if (pressure < 0) bloodPressure = 0;
	else if (pressure > maxBloodPressure) bloodPressure = maxBloodPressure;
	else bloodPressure = pressure;
}

void Entity::setMaxBloodPressure(const double pressure) noexcept {
	if (pressure < 0) maxBloodPressure = 0;
	else maxBloodPressure = pressure;
}

void Entity::tick() noexcept(false) {
	updatePosition();
	if (!world) return;
	if (accelerate.getX() == 0) accelerate.setX(accelerate.getX() + (isOnGround() ? nRange(-velocity.getX(), -0.2, 0.2) : nRange(-velocity.getX(), -0.03, 0.03)));
	accelerate.add(world->getForceField(*this, !cancelGravityOnce));
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
	momentum.locationTick = game.getWorldManager().getTick();
	momentum.velocity.periods.clear();
	momentum.atomicRelease();
}

void Entity::changeWorld(const WorldID id, const bool discardMovements) noexcept {
	momentum.atomicAcquire();
	momentum.location.setWorld(id);
	if (discardMovements) {
		momentum.locationTick = game.getWorldManager().getTick();
		momentum.velocity.periods.clear();
	}
	momentum.atomicRelease();
}
