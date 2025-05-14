//
// Created by EmsiaetKadosh on 25-4-25.
//

#include "Block.h"
#include "World.h"
#include "..\Game.h"

bool Block::adaptEntityVelocity(Entity& entity, const Vector2D& position, Vector2D& velocity, const Vector2D& rest, Vector2D& currentRest, const CollidingSide side) const {
	Vector2D adapted = velocity;
	switch (side) {
		case CollidingSide::LEFT_TOP: {
			const CollidingSide sd = CollidingSide::fromVector2D(velocity.getFourWay());
			if (sd == CollidingSide::RIGHT) goto left;
			if (sd == CollidingSide::BOTTOM) goto top;
			Logger.error(getLocation().toString());
			Logger.error(side.toString() + L" => " + sd.toString());
			break;
		}
		case CollidingSide::LEFT_BOTTOM: {
			const CollidingSide sd = CollidingSide::fromVector2D(velocity.getFourWay());
			if (sd == CollidingSide::RIGHT) goto left;
			if (sd == CollidingSide::TOP) goto bottom;
			Logger.error(getLocation().toString());
			Logger.error(side.toString() + L" => " + sd.toString());
			break;
		}
		case CollidingSide::RIGHT_TOP: {
			const CollidingSide sd = CollidingSide::fromVector2D(velocity.getFourWay());
			if (sd == CollidingSide::LEFT) goto right;
			if (sd == CollidingSide::BOTTOM) goto top;
			Logger.error(getLocation().toString());
			Logger.error(side.toString() + L" => " + sd.toString());
			break;
		}
		case CollidingSide::RIGHT_BOTTOM: {
			const CollidingSide sd = CollidingSide::fromVector2D(velocity.getFourWay());
			if (sd == CollidingSide::LEFT) goto right;
			if (sd == CollidingSide::TOP) goto bottom;
			Logger.error(getLocation().toString());
			Logger.error(side.toString() + L" => " + sd.toString());
			break;
		}
		case CollidingSide::LEFT: {
		left:
			adapted.extendValueX(static_cast<double>(location.getX()) - entity.getBoundingBox().getRight() - position.getX() /* - EpsilonValue */);
			currentRest.strictSelect((rest - adapted).setX(0));
			if (adapted.lengthManhattan() >= velocity.lengthManhattan()) return false;
			return velocity = adapted, true;
		}
		case CollidingSide::TOP: {
		top:
			adapted.extendValueY(static_cast<double>(location.getY()) - entity.getBoundingBox().getBottom() - position.getY() /* - EpsilonValue */);
			currentRest.strictSelect((rest - adapted).setY(0));
			if (adapted.lengthManhattan() >= velocity.lengthManhattan()) return false;
			return velocity = adapted, true;
		}
		case CollidingSide::RIGHT: {
		right:
			adapted.extendValueX(static_cast<double>(location.getX()) + 1 + entity.getBoundingBox().getLeft() - position.getX() /* + EpsilonValue */);
			currentRest.strictSelect((rest - adapted).setX(0));
			if (adapted.lengthManhattan() >= velocity.lengthManhattan()) return false;
			return velocity = adapted, true;
		}
		case CollidingSide::BOTTOM: {
		bottom:
			adapted.extendValueY(static_cast<double>(location.getY()) + 1 + entity.getBoundingBox().getTop() - position.getY() /* + EpsilonValue */);
			currentRest.strictSelect((rest - adapted).setY(0));
			if (adapted.lengthManhattan() >= velocity.lengthManhattan()) return false;
			return velocity = adapted, true;
		}
		default: // COVER此处也不做处理，既然穿过来了那就穿过来了吧
			break;
	}
	return false;
}

void TimedBarrierBlock::render(double tickDelta, QWORD tickRendering) const noexcept {
	renderer.fillWorldBlock(getLocation(), 0xffee0000);
	Vector2D&& pos = renderer.world2client(getLocation().getPosition());
	renderer.getFontManager().getDefault().drawCenter(RenderableString(std::to_wstring(time)), static_cast<int>(pos.getX()), static_cast<int>(pos.getY()), static_cast<int>(interactSettings.actual.mapScale), static_cast<int>(interactSettings.actual.mapScale), 0xff000000);
}

void TimedBarrierBlock::tick() noexcept(false) {
	if (!--this->time)
		game.tasks.pushNewed(Task::of([this](Task&) {
			if (game.worldManager->getWorld(getLocation().getWorld())->removeBlock(this, WorldTransportReason::BlockBreak)) Logger.warn(L"Failed to remove !");
			else onRemove();
		}));
}
