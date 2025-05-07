//
// Created by EmsiaetKadosh on 25-4-25.
//

#include "Block.h"
#include "World.h"
#include "..\Game.h"

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
