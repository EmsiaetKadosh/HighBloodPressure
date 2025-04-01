//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

// #include "..\utils\gc.h"
#include "..\ui\Hud.h"
#include "..\utils\Task.h"
#include "..\ui\Window.h"

class [[carlbeks::predecl, carlbeks::defineat("World.h")]] WorldManager;
class [[carlbeks::predecl, carlbeks::defineat("Entity.h")]] EntityManager;

class Game final /* : public IRenderable, public ITickable */ {
	friend void gameThread();
	friend void renderThread();
	WindowManager windows;
	Hud hud = Hud(); // 8
	CaptionWindow* caption; // 8
	FloatWindow* floatWindow; // 8
	QWORD currentTick = 0; // 8

public:
	TaskScheduler tasks; // 8
	std::minstd_rand random;
	WorldManager* worldManager = nullptr;
	EntityManager* entityManager = nullptr;

private:
	/**
	 * 此变量只用于在renderThread和gameThread中同步renderThread启动渲染瞬时获取的currentTick
	 */
	AtomicStorage<void> currentTickFlag;

public:
	void initialize();
	Game();
	~Game();

	int closeWindow(Window* const window) noexcept { return windows.pop(window); }
	[[nodiscard]] FloatWindow& getFloatWindow() const noexcept { return *floatWindow; }
	[[nodiscard]] QWORD getTick() const noexcept { return currentTick; }
	void tick() noexcept;
	void render(double tickDelta, QWORD tickRendering) const noexcept;

	/**
	 * 所有窗口都提交给Game保管，在适当时刻自动删除。
	 * 建议WindowType::create
	 */
	int setWindow(Window* window) noexcept {
		if (window) {
			if (window->onOpen()) {
				for (Window& i : windows) i.passEvent(MouseActionCode::MAC_LEAVE, 0, 0, 0);
				windows.pushNewed(window);
				Success();
			}
			Failed();
		}
		windows.clear();
		Success();
	}

	[[nodiscard]] Window* getWindow() const noexcept {
		if (auto* const back = windows.back()) return dynamic_cast<Window*>(back);
		return nullptr;
	}

	void handleResize() {
		caption->onResize();
		hud.onResize();
		windows.onResize();
		floatWindow->onResize();
	}

	int passEvent(const MouseActionCode action, const MouseButtonCode value, const int x, const int y) const noexcept {
		int ret = 0;
		ret = caption->passEvent(action, value, x, y);
		if (Window* const window = getWindow()) window->passEvent(action, value, x, y);
		floatWindow->passEvent(action, value, x, y);
		return ret;
	}
};

extern Game game;
