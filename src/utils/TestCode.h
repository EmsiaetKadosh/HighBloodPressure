//
// Created by EmsiaetKadosh on 25-1-20.
//

#pragma once

inline void test() {
	World* startWorld = game.worldManager->getWorld(1);
	const RayTraceResults& results = startWorld->rayTraceBlocks(Vector2D(0, 0), Vector2D(-5, 5));
	for (const RayTraceBlockResult& result : results.locations) {
		Logger.info(result.getBlockLocation().getPosition().toString() + L" " + result.getHitPoint().toString() + L" " + result.getHitSide().toString());
	}
	constexpr QWORD i = sizeof(Time);
}
