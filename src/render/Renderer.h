//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#include "..\def.h"
#include "..\utils\math.h"

#include "..\utils\Task.h"
#include "..\hbp.h"
#include "..\game\Animation.h"

#include "..\interact\InteractManager.h"
#include "..\render\FontManager.h"
#include "..\game\world\Location.h"
#include "TextureManager.h"

class Game;

/**
 * 用于标记相对位置。
 */
enum class UILocation : char { LEFT_TOP, LEFT, LEFT_BOTTOM, TOP, CENTER, BOTTOM, RIGHT_TOP, RIGHT, RIGHT_BOTTOM };

struct IRenderable {
	virtual ~IRenderable() = default;
	virtual void render(double tickDelta, QWORD tickRendering) const noexcept = 0;
};

struct ITickable {
	virtual ~ITickable() = default;
	virtual void tick() noexcept(false) = 0;
};

struct Color {
	unsigned int inactive = 0xff777777;
	unsigned int active = 0xff000000;
	unsigned int hover = 0xff444444;
	unsigned int clicked = 0xffeeeeee;
};

inline static constexpr Color TextColor = {
	.inactive = 0xff333333,
	.active = 0xffeeeeee,
	.hover = 0xffeeeeee,
	.clicked = 0xff000000
};

class [[carlbeks::predecl, carlbeks::defineat("Entity.h")]] Entity;

class Camera {
	Vector2D position = Vector2D(); // 当前位置
	Vector2D targetPosition = Vector2D(); // Camera需要移动到的实体的位置
	Entity* renderingTarget = nullptr;
	Entity* targeting = nullptr;
	QWORD entityUpdateTick = 0;
	QWORD positionUpdateTick = 0; // 用于判断Camera的位置更新，实现渲染线程计算但是与游戏线程在时间上同步
	double positionUpdateTickDelta = 0; // 用于判断Camera的位置更新，实现渲染线程计算但是与游戏线程在时间上同步

public:
	void render(double tickDelta, QWORD tickRendering) noexcept;
	void setTargetEntity(Entity* target) noexcept;
	[[nodiscard]] Entity* getTargetingEntity() const noexcept { return targeting; }
	[[nodiscard]] Entity* getRenderingTargetEntity() const noexcept { return renderingTarget; }
	[[nodiscard]] double getCurrentX() const noexcept { return position.getX(); }
	[[nodiscard]] double getCurrentY() const noexcept { return position.getY(); }
	[[nodiscard]] double getTargetX() const noexcept { return targetPosition.getX(); }
	[[nodiscard]] double getTargetY() const noexcept { return targetPosition.getY(); }
	[[nodiscard]] Vector2D getCurrentPosition() const noexcept { return position; }
	[[nodiscard]] Vector2D getTargetPosition() const noexcept { return targetPosition; }
};

class IRenderer : public ITickable {
protected:
	int windowWidth = 0, windowHeight = 0;
	int syncWidth = 0, syncHeight = 0; // 指示实时大小。为了防抖，只会在改变窗口大小结束时resize并重写windowWidth和windowHeight
	mutable Camera camera;
	Animation mousePointingAtFlash = Animation().includeReverse().setDuration(40).features(Animation::AS_QUADRATIC).depends(Animation::AD_TIME);
	Task resizeReloadBitmap { nullptr };

	/**
	 * @brief 辅助函数，传递friend属性。用于在resize中设置uiScale和mapScale而不requireResize
	 */
	void setSystemScale() const noexcept { interactSettings.resizeSetSystemScale(nMin(static_cast<double>(windowWidth) / 3840.0, static_cast<double>(windowHeight) / 2160.0)); }
	/**
	 * @brief 辅助函数，传递friend属性。用于解包TextureEntry中的ITexture*
	 */
	static ITexture* textureOf(TextureEntry& entry) noexcept { return entry.texture; }

public:
	double fps = 0, tps = 0;
	Vector2D mousePointingAtWorld = Vector2D();
	BlockLocation mousePointingAtBlock = BlockLocation(0, 0, 0);

protected:
	bool zoomed = false;

public:
	IRenderer() { initializerChecker.requiredModule(L"Renderer", L"Game").registerModule(L"Renderer"); }
	IRenderer(const IRenderer&) = delete;
	IRenderer(IRenderer&&) = delete;
	IRenderer& operator=(const IRenderer&) = delete;
	IRenderer& operator=(IRenderer&&) = delete;
	~IRenderer() override = default;

	/**
	 * @brief 创建实例后，必须调用
	 */
	virtual IRenderer& postInitialize() noexcept = 0;

	void syncSize(const int width, const int height) noexcept { syncWidth = width, syncHeight = height; }
	void setZoom(const bool value) noexcept { if (zoomed != value) zoomed = value, ShowWindow(MainWindowHandle, value ? SW_MAXIMIZE : SW_RESTORE); }
	bool isZoomed() const noexcept { return zoomed; }
	[[nodiscard]] int getWidth() const noexcept { return windowWidth; }
	[[nodiscard]] int getHeight() const noexcept { return windowHeight; }
	[[nodiscard]] int getClientHeight() const noexcept;
	[[nodiscard]] int getSyncWidth() const noexcept { return syncWidth; }
	[[nodiscard]] int getSyncHeight() const noexcept { return syncHeight; }
	[[nodiscard]] Camera& getCamera() const { return camera; }

	[[nodiscard]] virtual bool checkResizing() const = 0;
	[[nodiscard]] virtual unsigned int changeColorFormat(unsigned int argb) const noexcept = 0;
	virtual void finalize(bool isRenderThread) = 0;
	virtual void gameStartRender() = 0;
	virtual void gameEndRender() = 0;
	virtual void requireResize() = 0;
	virtual void resize(int width, int height) = 0;
	virtual void assertRendering() const = 0;
	virtual void assertRenderThread() const = 0;
	virtual void resizeStart() = 0;
	virtual void resizeShow() const = 0;
	virtual void resizeEnd() = 0;
	virtual void renderMouseWorld() = 0;
	Vector2D world2client(Vector2D world) const { return world.subtract(camera.getCurrentPosition()).multiply(interactSettings.actual.mapScale).add(windowWidth >> 1, windowHeight >> 1); }
	Vector2D client2world(const int x, const int y) const { return Vector2D(x - (windowWidth >> 1), y - (windowHeight >> 1)).divide(interactSettings.actual.mapScale).add(camera.getCurrentPosition()); }

	virtual void fill(int x, int y, int w, int h, unsigned int color) const = 0;
	virtual void fill(const RECT* rect, unsigned int color) const = 0;
	virtual void fillWorld(const Vector2D& from, const Vector2D& to, unsigned int color) const = 0;
	virtual void fillWorld(const Vector2D& from, double blockWidth, double blockHeight, unsigned int color) const = 0;
	virtual void fillWorldBlock(const BlockLocation& from, unsigned int color) const = 0;
	virtual void texture(TextureEntry, int x, int y, int w, int h) const noexcept = 0;
	virtual void textureWorld(TextureEntry entry, Vector2D from, Vector2D targetSizeBlock = Vector2D(1, 1), unsigned char alpha = 255, POINT srcFrom = {}, POINT srcSize = {}) const noexcept = 0;
};
