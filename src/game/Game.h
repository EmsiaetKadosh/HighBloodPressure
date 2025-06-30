//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#include "..\global.hpp"

// #include "..\utils\gc.h"
#include "..\ui\Hud.h"
#include "..\utils\Task.h"
#include "..\ui\Window.h"

class [[carlbeks::predecl, carlbeks::defineat("World.h")]] WorldManager;
class [[carlbeks::predecl, carlbeks::defineat("Entity.h")]] Entity;
class [[carlbeks::predecl, carlbeks::defineat("Entity.h")]] EntityManager;
class [[carlbeks::predecl, carlbeks::defineat("Block.h")]] Block;

struct GameOptions {
	struct Resource {
		bool lazyLoad = false;
	} resource;

	struct Operations {
		bool autoJumpHighest = true;
	} operations;

	struct Gaming {
		bool timeFreeze = false;
		byte timeFreezeRatio = 10;
	} gaming;
};

class Game final /* : public IRenderable, public ITickable */ {
	friend void gameThread();
	friend void renderThread();
	WindowManager windows;
	Hud hud = Hud(); // 8
	CaptionWindow* caption; // 8
	FloatWindow* floatWindow; // 8
	WorldManager* worldManager = nullptr;
	EntityManager* entityManager = nullptr;
	std::minstd_rand random;
	QWORD currentTick = 0; // 8

public:
	TaskScheduler tasks; // 8
	GameOptions options;

private:
	/**
	 * 此变量只用于在renderThread和gameThread中同步renderThread启动渲染瞬时获取的currentTick
	 */
	AtomicStorage currentTickFlag;
	byte tickFreeze = 0;

public:
	void initialize();
	Game();
	~Game();

	int closeWindow(Window* const window) noexcept { return windows.pop(window); }
	[[nodiscard]] bool isReady() const noexcept { return worldManager && entityManager; }
	[[nodiscard]] WorldManager& getWorldManager() const noexcept { return *worldManager; }
	[[nodiscard]] EntityManager& getEntityManager() const noexcept { return *entityManager; }
	[[nodiscard]] QWORD nextRandom() const noexcept { return random, 0; }
	[[nodiscard]] FloatWindow& getFloatWindow() const noexcept { return *floatWindow; }
	[[nodiscard]] CaptionWindow& getCaption() const noexcept { return *caption; }
	[[nodiscard]] QWORD getTick() const noexcept { return currentTick; }
	void tick() noexcept(false);
	void render(double tickDelta, QWORD tickRendering) const noexcept;
	void handleResize();
	int passEvent(MouseActionCode action, MouseButtonCode value, int x, int y) const noexcept;

	/**
	 * 所有窗口都提交给Game保管，在适当时刻自动删除。
	 * 建议WindowType::create
	 * @param window 要打开的窗口，nullptr表示清空窗口且popFailed参数无意义
	 * @param popFailed 是否在失败时自动调用pop
	 */
	int setWindow(Window* window, bool popFailed = true) noexcept;

	[[nodiscard]] Window* getWindow() const noexcept {
		if (auto* const back = windows.back()) return dynamic_cast<Window*>(back);
		return nullptr;
	}


	template <typename T, typename... Args> requires requires {
		std::is_base_of_v<Entity, T> || std::is_base_of_v<Block, T>;
		new T(std::declval<Args>()...);
	}
	T* newInstanceOf(Args&&... args) const noexcept { return allocatedFor(new T(std::forward<Args>(args)...)); }
};
