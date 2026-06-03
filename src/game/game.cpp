
#include <chrono>

#include "src\main.hpp"
#include "src\windows.hpp"
#include "src\render\dx\direct.hpp"
#include "src\utils\exception.hpp"
#include "src\utils\time.hpp"

class GameImpl final : public Game {
	GameCrashRiskManager riskManager;
	InteractManager interactManager;
	TextureManager textureManager;
	DirectRenderer renderer;
	Renderer renderInterface { &renderer };
	Camera cameraInterface { &renderer.getCamera() };
	Thread gameThread, renderThread;
	Average<64> averageTickTimeCost;
	Average<64> averageRenderTimeCost;
	long long nanosecondsPerTick = 8'000'000;
	long long nanosecondsPerRender = 8'000'000;
	Timer tickTimer;
	Timer renderTimer;

	static void GameThread() {
		Logger.of(L"GameThread started. id: ", std::this_thread::get_id()).info();
		GameImpl& impl = static_cast<GameImpl&>(game); // NOLINT(*-pro-type-static-cast-downcast)
		try { while (impl.isRunning) impl.tick(); }
		catch (Exception& e) { e.printStacktrace().crash(L"Unhandled GameThread exception"); }
		Logger.warn(L"GameThread ended");
		impl.isRunning = false;
	}

	static void RenderThread() {
		Logger.of(L"RenderThread started. id: ", std::this_thread::get_id()).info();
		GameImpl& impl = static_cast<GameImpl&>(game); // NOLINT(*-pro-type-static-cast-downcast)
		try {
			impl.renderer.declareThread();
			while (impl.isRunning) impl.render();
		}
		catch (Exception& e) { e.printStacktrace().crash(L"Unhandled RenderThread exception"); }
		Logger.warn(L"RenderThread ended");
	}

	static void startThreads(GameImpl& This) noexcept {
		This.gameThread = Thread(GameThread);
		This.renderThread = Thread(RenderThread);
		This.gameThread.detach();
		This.renderThread.detach();
	}

	static void terminateThreads(GameImpl& This) noexcept {
		if (This.gameThread.joinable()) This.gameThread.join();
		if (This.renderThread.joinable()) This.renderThread.join();
	}

public:
	GameImpl() = default;

	void initialize() noexcept override {
		Logger.log(L"Initializing...");
		if (renderer.initialize(MainWindowHandle)) return isRunning = false, void();
		startThreads(*this);
		Logger.log(L"Game initialized");
	}

	void finalize() noexcept override {
		isRunning = false;
		terminateThreads(*this);
		renderer.finalize();
	}

	void crash(String str) noexcept override {
		isRunning = false;
		Logger.error(std::move(str));
		// TODO(EmsiaetKadosh): 单独崩溃日志输出
	}

	void stop() noexcept override {
		isRunning = false;
		Logger.warn(L"Game terminated");
		// Logger.ofNoexcept(L"DestroyWindow => ", DestroyWindow(MainWindowHandle)).debug(); 不可跨线程摧毁
		PostMessageW(MainWindowHandle, WM_APP_TERMINATE, 0, 0);
	}

	void tick() noexcept(false) override {
		++tickCount;
		// TODO(EmsiaetKadosh): 其他逻辑

		Vector3D v = getCamera().getDirection();
		Vector3D res = Vector3D();
		if (interactManager.peek(VK_ESCAPE)) game.stop();
		if (interactManager.peek(VK_SPACE)) res.add(0, 1, 0);
		if (interactManager.peek(VK_SHIFT)) res.add(0, -1, 0);
		if (interactManager.peek('W')) {
			v.setY(0);
			v.normalize();
			res.add(v);
		}
		if (interactManager.peek('S')) {
			v.setY(0);
			v.normalize();
			res.subtract(v);
		}
		if (interactManager.peek('A')) res.add(Vector3D(0, 1, 0).cross(v).normalize());
		if (interactManager.peek('D')) res.add(Vector3D(0, -1, 0).cross(v).normalize());
		if (res.lengthManhattan() != 0) getCamera().move(res.normalize().multiply(0.02));

		if (interactManager.consume('R')) cameraInterface.useProjectionSwitch();

		tickTimer.waitUntilAndReset(nanosecondsPerTick);
		averageTickTimeCost.push(static_cast<double>(tickTimer.lap()));
		if (averageTickTimeCost.isFull()) { // TODO(EmsiaetKadosh): debug
			Logger.of(L"Tick average: ", averageTickTimeCost.get() * 0.000'001).debug();
			averageTickTimeCost.clear();
		}
	}

	void render() noexcept(false) override {
		if (true || stp) {
			renderer.begin();
			// TODO(EmsiaetKadosh): 其他逻辑
			renderer.drawColor(
				ColoredSetBuilder()
				.predictPolygonEdges(3)
				.nextColored(0xffeeeeee).thisAt(1.0f, 0.0f, 2.0f) // left
				.nextColored(0xffee0000).thisAt(-1.0f, 0.0f, 2.0f) // right red
				.nextColored(0xffeeeeee).thisAt(0.0f, 1.0f, 2.0f) // top
				.build());

			renderer.drawColor(
				ColoredSetBuilder()
				.predictPolygonEdges(6)
				.nextColored(0xffee0000).thisAt(0.0f, 0.0f, 0.0f)
				.nextColored(0xffee0000).thisAt(10.0f, 0.1f, 0.0f)
				.nextColored(0xffee0000).thisAt(10.0f, 0.0f, 0.1f)
				.nextColored(0xffee0000).thisAt(10.0f, -0.1f, 0.0f)
				.nextColored(0xffee0000).thisAt(10.0f, 0.0f, -0.1f)
				.nextColored(0xffee0000).thisAt(10.0f, 0.1f, 0.0f)
				.build());
			renderer.drawColor(
				ColoredSetBuilder()
				.predictPolygonEdges(6)
				.nextColored(0xff00ee00).thisAt(0.0f, 0.0f, 0.0f)
				.nextColored(0xff00ee00).thisAt(0.0f, 10.0f, 0.1f)
				.nextColored(0xff00ee00).thisAt(0.1f, 10.0f, 0.0f)
				.nextColored(0xff00ee00).thisAt(0.0f, 10.0f, -0.1f)
				.nextColored(0xff00ee00).thisAt(-0.1f, 10.0f, 0.0f)
				.nextColored(0xff00ee00).thisAt(0.0f, 10.0f, 0.1f)
				.build());
			renderer.drawColor(
				ColoredSetBuilder()
				.predictPolygonEdges(6)
				.nextColored(0xff0000ee).thisAt(0.0f, 0.0f, 0.0f)
				.nextColored(0xff0000ee).thisAt(0.1f, 0.0f, 10.0f)
				.nextColored(0xff0000ee).thisAt(0.0f, 0.1f, 10.0f)
				.nextColored(0xff0000ee).thisAt(-0.1f, 0.0f, 10.0f)
				.nextColored(0xff0000ee).thisAt(0.0f, -0.1f, 10.0f)
				.nextColored(0xff0000ee).thisAt(0.1f, 0.0f, 10.0f)
				.build());
			renderer.end();
			stp = false;
		}
		renderTimer.waitUntilAndReset(nanosecondsPerRender);
		averageRenderTimeCost.push(static_cast<double>(renderTimer.lap()));
		if (averageRenderTimeCost.isFull()) { // TODO(EmsiaetKadosh): debug
			Logger.of(L"Render average: ", averageRenderTimeCost.get() * 0.000'001).debug();
			averageRenderTimeCost.clear();
		}
	}

	GameCrashRiskManager& getRiskManager() noexcept override { return riskManager; }
	InteractManager& getInteractManager() noexcept override { return interactManager; }
	TextureManager& getTextureManager() noexcept override { return textureManager; }
	Renderer& getRenderer() noexcept override { return renderInterface; }
	Camera& getCamera() noexcept override { return cameraInterface; }
	void step() noexcept override { stp = true; }

	bool stp = false;
};

Game& Details::getGame() {
	static GameImpl impl;
	return impl;
}
