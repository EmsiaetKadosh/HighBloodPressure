//
// Created by EmsiaetKadosh on 25-1-14.
//

#include "Renderer.h"

#include "..\game\Game.h"
#include "..\game\entity\Entity.h"
#include "..\interact\InteractManager.h"

void Camera::render(const double tickDelta, const QWORD tickRendering) noexcept {
	renderingTarget = targeting;
	if (renderingTarget) {
		const Vector2D originalRelative = position - targetPosition;
		renderingTarget->getMomentum().atomicAcquire();
		targetPosition = renderingTarget->getLocation(tickDelta, tickRendering).getPosition();
		renderingTarget->getMomentum().atomicRelease();
		Vector2D newRelative = renderingTarget->getVelocity() + renderingTarget->getAcceleration();
		Vector2D selector = Vector2D(static_cast<double>(renderer.getWidth()) / interactSettings.actual.mapScale, static_cast<double>(renderer.getHeight()) / interactSettings.actual.mapScale);
		selector.multiply(0.2);
		newRelative.setX(newRelative.getX() * 5);
		newRelative.setY(newRelative.getY());
		newRelative.setX(nRangeSmooth(newRelative.getX(), -selector.getX(), selector.getX()));
		newRelative.setY(nRangeSmooth(newRelative.getY(), -selector.getY(), selector.getY()));
		Vector2D movement = newRelative - originalRelative;
		movement.multiply(1 - interactSettings.constants.smoothCamera);
		if (selector.lengthManhattan() != 0) {
			const double eff = nRangeSmooth(originalRelative.length() / selector.length(), -0.6, 1.3);
			movement.multiply(eff);
		}
		if (tickRendering == positionUpdateTick) { // 实现渲染线程计算，时间上与游戏时间同步
			movement.multiply(tickDelta - positionUpdateTickDelta);
			positionUpdateTickDelta = tickDelta;
		}
		else {
			movement.multiply(tickDelta - positionUpdateTickDelta + tickRendering - positionUpdateTick); // 补足上一tick未完成的计算
			positionUpdateTick = tickRendering;
			positionUpdateTickDelta = tickDelta;
		}
		position = targetPosition + originalRelative + movement;
	}
}

void Camera::setTargetEntity(Entity* target) noexcept {
	entityUpdateTick = game.getTick();
	targeting = target;
}

int IRenderer::getClientHeight() const noexcept { return game.getCaption().isHidden() ? windowHeight : windowHeight - interactSettings.actual.captionHeight; }


// #define __CARLBEKS_USE_DX__
#if defined __CARLBEKS_USE_DX__
#include "dx\direct.h"
#endif
