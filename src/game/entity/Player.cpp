//
// Created by EmsiaetKadosh on 25-3-21.
//

#include "Player.h"
Player::Player(KeyRegion& kr, const Vector2D& location): Entity(location), left(*kr.getBinding(L"move_left")), right(*kr.getBinding(L"move_right")), jump(*kr.getBinding(L"move_jump")), dodge(*kr.getBinding(L"move_dodge")) {}

Player::Player(const Vector2D& location): Player(interactManager.getKeyBindingManager().getRegion(L"world"), location) {
	boundingBox.setLeft(0.6);
	boundingBox.setRight(0.6);
	boundingBox.setTop(2.5);
	maxSpeed = 0.3;
}

void Player::processKey() {
	if (jump.isPressed()) {
		if (jumpInterval) --jumpInterval;
		else if (!game.options.operations.autoJumpHighest || velocity.getY() >= 0 || jump.wasPressedThenDeal()) { // 这里是一个自动高跳的判断
			if (isOnGround()) accelerate.setY(-0.4);
			else if (airJump == 2) velocity.setY(-0.25), accelerate.setY(0), --airJump;
			else if (airJump == 1) {
				--airJump, velocity.setY(-0.4), accelerate.setY(0);
				if (left.isPressedThenDeal()) velocity.setX(nMin(-maxSpeed, velocity.getX()));
				if (right.isPressedThenDeal()) velocity.setX(nMax(+maxSpeed, velocity.getX()));
			}
			jumpInterval = 3;
		}
		jump.deals(); // 此处延迟deal。因为在此if内部，有一行jump.wasPressedThenDeal()，不能在开始时就isPressedThenDeal()
	}
	if (left.isPressedThenDeal()) accelerate.add(-0.02, 0);
	if (right.isPressedThenDeal()) accelerate.add(0.02, 0);
	if (accelerate.getX() != 0) accelerate.setX(nRange(accelerate.getX(), (-maxSpeed - velocity.getX()) * 0.2, (maxSpeed - velocity.getX()) * 0.2));
	jump.deals();
}

void Player::tick() noexcept(false) {
	updatePosition();
	accelerate = Vector2D();
	bool shouldProcessKeys = true;
	if (dodgeInterval > 0) --dodgeInterval;
	else if (dodgeInterval < 0) {
		shouldProcessKeys = false;
		if (dodgeDirection == 0) Logger.warn(L"DodgeDirection = 0");
		else velocity.setX(dodgeDirection * maxSpeed * 1.5);
		Logger.debug(L"Dodging");
		++dodgeInterval;
	}
	else if (dodge.isPressedThenDeal()) {
		dodgeDirection = 0;
		if (left.isPressedThenDeal()) --dodgeDirection;
		if (right.isPressedThenDeal()) ++dodgeDirection;
		if (dodgeDirection) dodgeInterval = -8;
		else { // 按键无法决定方向
			if (velocity.getX() < 0) dodgeDirection = -1;
			if (velocity.getX() > 0) dodgeDirection = 1;
			if (dodgeDirection) dodgeInterval = -8;
		}
	}
	if (shouldProcessKeys) processKey();
	Entity::tick();
}

void Player::checkOnGround() noexcept {
	Entity::checkOnGround();
	if (onGround) airJump = 2;
}

void Player::render(const double tickDelta, const QWORD tickRendering) const noexcept {
	if (renderer.getCamera().getRenderingTargetEntity() == this) {
		renderer.fillWorld(renderer.getCamera().getTargetPosition().add(boundingBox.getLeftTopOffset()), boundingBox.getWidth(), boundingBox.getHeight(), airJump == 2 ? 0xff44ee66 : airJump == 1 ? 0xffeeee66 : 0xffdd7755);
		return;
	}
	momentum.atomicAcquire();
	renderer.fillWorld(getLocation(tickDelta, tickRendering).getPosition().add(boundingBox.getLeftTopOffset()), boundingBox.getWidth(), boundingBox.getHeight(), airJump == 2 ? 0xff44ee66 : airJump == 1 ? 0xffeeee66 : 0xffdd7755);
	momentum.atomicRelease();
}
