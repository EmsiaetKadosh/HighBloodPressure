//
// Created by EmsiaetKadosh on 25-1-14.
//

#include "Game.h"

#include "entity\Entity.h"
#include "world\World.h"
#include "..\ui\xWindows.h"

void Game::initialize() {
	worldManager = allocatedFor(new WorldManager);
	entityManager = allocatedFor(new EntityManager);
	// setWindow(StartWindow::create());
	windows.onResize();
}

Game::Game() : caption{ allocatedFor(new CaptionWindow()) }, floatWindow{ allocatedFor(new FloatWindow()) } {
	Logger.put(L"Game created");
	random.seed(static_cast<unsigned int>(getCurrentTime().time_since_epoch().count()));
}

Game::~Game() {
	setWindow(nullptr);
	delete deallocating(floatWindow);
	gc.pack();
	gc.collect();
	delete deallocating(caption);
	gc.pack();
	gc.collect();
	delete deallocating(worldManager);
	gc.pack();
	gc.collect();
	delete deallocating(entityManager);
	gc.pack();
	gc.collect();
}

void Game::render(const double tickDelta, const QWORD tickRendering) const noexcept {
	if (renderer.checkResizing()) return;
	renderer.getCamera().render(tickDelta, tickRendering);
	renderer.gameStartRender();
	if (worldManager->current) {
		worldManager->current->render(tickDelta, tickRendering);
		renderer.renderMouseWorld();
	}
	caption->render(tickDelta, tickRendering);
	hud.render(tickDelta, tickRendering);
	windows.render(tickDelta, tickRendering);
	floatWindow->render(tickDelta, tickRendering);
	renderer.gameEndRender();
	gc.pack();
}

void Game::tick() noexcept(false) {
	// ++currentTick; // 托管到gameThread完成
	floatWindow->clear();
	floatWindow->tick();
	renderer.tick();
	worldManager->tick();
	caption->tick();
	hud.tick();
	windows.tick();
	floatWindow->update();
	tasks.runAll();
	gc.collect();
}

inline Game game = Game();
