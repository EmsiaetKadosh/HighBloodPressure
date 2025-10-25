
#pragma once
#include "src\game\interact\interact.hpp"
#include "src\render\render.hpp"

class Game;

namespace Details {
	extern Game& getGame();
}

class Game {
protected:
	static Game* instance;
	unsigned long long tickCount = 0;
	bool isRunning = true;
	bool isInitialized = false;

	Game() = default;

public:
	static Game& getInstance() { return *instance; }
	Game(const Game&) = delete;
	Game(Game&&) = delete;
	Game& operator=(const Game&) = delete;
	Game& operator=(Game&&) = delete;
	virtual ~Game() noexcept = default;
	virtual void initialize() noexcept = 0;
	virtual void finalize() noexcept = 0;
	virtual void crash(String) noexcept = 0;
	virtual void stop() noexcept = 0;
	virtual void tick() noexcept(false) = 0;
	virtual void render() noexcept(false) = 0;

	virtual InteractManager& getInteractManager() noexcept = 0;
	virtual Renderer& getRenderer() noexcept = 0;
	virtual Camera& getCamera() noexcept = 0;
	virtual void step() noexcept = 0;

	[[nodiscard]] bool isReady() const noexcept { return isRunning && isInitialized; }
	[[nodiscard]] unsigned long long getTick() const noexcept { return tickCount; }
};
