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
	static unsigned int logCount;
	if (renderer.checkResizing()) {
		if (!--logCount) logCount = 30, Logger.trace(L"waiting for resize");
		return;
	}
	if (logCount != 30) logCount = 30, Logger.trace(L"render (resize completed)");
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

void Game::handleResize() {
	caption->onResize();
	hud.onResize();
	windows.onResize();
	floatWindow->onResize();
}

int Game::passEvent(const MouseActionCode action, const MouseButtonCode value, const int x, const int y) const noexcept {
	int ret = 0;
	ret = caption->passEvent(action, value, x, y);
	if (Window* const window = getWindow()) window->passEvent(action, value, x, y);
	floatWindow->passEvent(action, value, x, y);
	return ret;
}

int Game::setWindow(Window* window, const bool popFailed) noexcept {
	if (window) {
		if (window->onOpen()) {
			for (Window& i : windows) i.passEvent(MouseActionCode::MAC_LEAVE, 0, 0, 0);
			windows.pushNewed(window);
			Success();
		}
		if (popFailed) window->pop();
		Failed();
	}
	windows.clear();
	Success();
}

void Game::tick() noexcept(false) {
	// ++currentTick; // 托管到gameThread完成
	floatWindow->clear();
	floatWindow->tick();
	renderer.tick();
	if (!windows.pausesGame()) worldManager->tick();
	caption->tick();
	hud.tick();
	windows.tick();
	tasks.runAll();
	gc.collect();
}

inline Game game = Game();
