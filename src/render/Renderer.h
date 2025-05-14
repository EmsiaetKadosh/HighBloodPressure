//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#include "..\def.h"
#include "..\utils\math.h"
#include "..\utils\exception.h"
#include "..\utils\Chars.h"

#include "..\utils\Task.h"
#include "..\hbp.h"
#include "..\game\Animation.h"

#include "..\interact\InteractManager.h"
#include "..\utils\IText.h"
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
	QWORD updateTick = 0;

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
	mutable IFontManager* fontManager;
	Animation mousePointingAtFlash = Animation().includeReverse().setDuration(40).features(Animation::AS_QUADRATIC).depends(Animation::AD_TIME);
	Task resizeReloadBitmap{nullptr};

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
	IRenderer() = default;
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
	[[nodiscard]] IFontManager& getFontManager() const noexcept { return *fontManager; }
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

class GdiRenderer final : public IRenderer {
	friend class Game;
	friend class GdiFont;
	friend class GdiTexture;
	friend class GdiTextureManager;
	friend LRESULT __stdcall WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	inline static BLENDFUNCTION blendFunction = {
		.BlendOp = AC_SRC_OVER, // Only
		.BlendFlags = 0, // Must 0
		.SourceConstantAlpha = 255, // 预乘
		.AlphaFormat = 0, // Not AC_SRC_ALPHA
	};
	inline static BLENDFUNCTION blendFunctionSrcAlpha{
		.BlendOp = AC_SRC_OVER, // Only
		.BlendFlags = 0, // Must 0
		.SourceConstantAlpha = 255, // 预乘
		.AlphaFormat = 0, // Not AC_SRC_ALPHA
	};
	mutable List<HGDIOBJ> failed;
	HDC MainDC = nullptr; // 8
	HDC resizeCopyDC = nullptr; // 8
	HBITMAP resizeCopyBitmap = nullptr; // 8
	HDC canvasDC = nullptr; // 8
	HBITMAP canvasBitmap = nullptr; // 8
	HDC assistDC = nullptr; // 8
	HBITMAP assistBitmap = nullptr; // 8
	Thread::id renderThread = std::this_thread::get_id();
	Time lastPostRefreshTime = getCurrentTime(); // 8
	/**
	 * 缓存resizeCopyBitmap的宽高
	 */
	int resizeCopyWidth = 0, resizeCopyHeight = 0; // 4 + 4
	bool isRendering = false; // 1
	bool isResizing = false; // 1
	bool isResizeRequired = false; // 1
	char refreshedHDC = -1; // 1

	void gameStartRender() noexcept override;
	void gameEndRender() noexcept override;
	/**
	 * 负责转发所有resize信息
	 */
	void resize(int width, int height) noexcept(false) override;

	static bool $deleteObject(const HGDIOBJ obj) noexcept {
		const unsigned type = GetObjectType(obj);
		if (!type) return true;
		switch (type) {
			case OBJ_BITMAP:
				Logger.warn(L"DeleteObject failure: BITMAP");
				break;
			case OBJ_PEN:
				Logger.warn(L"DeleteObject failure: PEN");
				break;
			case OBJ_BRUSH:
				Logger.warn(L"DeleteObject failure: BRUSH");
				break;
			case OBJ_FONT:
				Logger.warn(L"DeleteObject failure: FONT");
				break;
			case OBJ_REGION:
				Logger.warn(L"DeleteObject failure: REGION");
				break;
			case OBJ_DC:
				Logger.warn(L"DeleteObject failure: DC");
				break;
			case OBJ_MEMDC:
				Logger.warn(L"DeleteObject failure: MEMDC");
				break;
			case OBJ_PAL:
				Logger.warn(L"DeleteObject failure: PAL");
				break;
			default:
				Logger.warn(L"DeleteObject failure: ? " + std::to_wstring(type));
				break;
		}
		return false;
	}

	void deleteObject(HGDIOBJ obj) const noexcept {
		List<HGDIOBJ> tempList;
		tempList.swap(failed);
		for (List<HGDIOBJ>::const_iterator iter = tempList.cbegin(); iter != tempList.cend(); ++iter)
			if (!DeleteObject(*iter)) {
				Logger.error(L"DeleteObject failed again. Deleting: " + qwtowb16(reinterpret_cast<QWORD>(*iter)) + L", LastError: " + std::to_wstring(GetLastError()));
				if ($deleteObject(obj)) Logger.info(L"DeleteObject failure: Invalid HGIDOBJ");
				else failed.push_back(*iter);
			}
		if (obj && !DeleteObject(obj)) {
			Logger.error(L"DeleteObject failed. Deleting: " + qwtowb16(reinterpret_cast<QWORD>(obj)) + L", LastError: " + std::to_wstring(GetLastError()));
			if ($deleteObject(obj)) Logger.info(L"DeleteObject failure: Invalid HGIDOBJ");
			else failed.push_back(obj);
		}
	}

	static void deleteDC(const HDC dc) noexcept {
		HGDIOBJ obj;
		if ((obj = GetCurrentObject(dc, OBJ_BITMAP))) SelectObject(dc, obj);
		if ((obj = GetCurrentObject(dc, OBJ_BRUSH))) SelectObject(dc, obj);
		if ((obj = GetCurrentObject(dc, OBJ_COLORSPACE))) SelectObject(dc, obj);
		if ((obj = GetCurrentObject(dc, OBJ_FONT))) SelectObject(dc, obj);
		if ((obj = GetCurrentObject(dc, OBJ_PAL))) SelectObject(dc, obj);
		if ((obj = GetCurrentObject(dc, OBJ_PEN))) SelectObject(dc, obj);
		DeleteDC(dc);
	}

public:
	GdiRenderer() {
		Logger.put(L"Renderer created");
		resizeReloadBitmap.func = [this](Task& task) {
			Logger.info(L"Scheduled task: resize reload bitmap " + std::to_wstring(windowWidth) + L" * " + std::to_wstring(windowHeight));
			if (!canvasBitmap) {
				canvasBitmap = CreateCompatibleBitmap(MainDC, windowWidth, windowHeight);
				if (canvasBitmap) SelectObject(canvasDC, canvasBitmap);
			}
			if (!assistBitmap) {
				assistBitmap = CreateCompatibleBitmap(canvasDC, windowWidth, windowHeight);
				if (assistBitmap) SelectObject(assistDC, assistBitmap);
			}
			if (canvasBitmap && assistBitmap) {
				task.pop();
				Logger.info(L"Successfully reload bitmap " + ptrtow(reinterpret_cast<QWORD>(canvasBitmap)) + L" " + ptrtow(reinterpret_cast<QWORD>(assistBitmap)));
				this->resizeEnd();
			}
		};
		textureManager.load();
	}

	~GdiRenderer() override {
		Logger.put(L"Renderer destroyed");
		// finalize();
		fontManager->finalize();
		delete fontManager;
	}

	IRenderer& postInitialize() noexcept override { return fontManager = new GdiFontManager(this), *this; }

	[[nodiscard]] bool checkResizing() const noexcept override { return isResizing; }
	/**
	 * @attention 会忽略A透明度值
	 * @param argb ARGB式颜色
	 * @return int BGR式颜色
	 */
	[[nodiscard]] unsigned int changeColorFormat(const unsigned int argb) const noexcept override { return argb << 16 & 0xff0000 | argb & 0xff00 | argb >> 16 & 0xff; }
	void requireResize() noexcept override { isResizeRequired = true; }
	void assertRendering() const noexcept(false) override { if (!isRendering) throw InvalidOperationException(L"Operation should be done while rendering"); }
	void assertRenderThread() const noexcept(false) override { if (std::this_thread::get_id() != renderThread) throw InvalidOperationException(L"Operation should be done in render thread"); }
	void resizeStart() noexcept override { isResizing = true; }
	void resizeShow() const noexcept override { StretchBlt(MainDC, 0, 0, syncWidth, syncHeight, resizeCopyDC, 0, 0, resizeCopyWidth, resizeCopyHeight, SRCCOPY); }
	void resizeEnd() noexcept override;
	void renderMouseWorld() noexcept override;
	void finalize(bool isRenderThread) noexcept override;
	void tick() noexcept(false) override;

	void fill(const int x, const int y, const int w, const int h, const unsigned int color) const override {
		assertRendering();
		//assertRenderThread();
		if ((color & 0xff000000) == 0) return;
		if ((color & 0xff000000) == 0xff000000) {
			const RECT rect{
				.left = x,
				.top = y,
				.right = x + w,
				.bottom = y + h
			};
			const HBRUSH clr = CreateSolidBrush(changeColorFormat(color));
			FillRect(canvasDC, &rect, clr);
			deleteObject(clr);
		}
		else {
			const RECT rect{
				.left = 0,
				.top = 0,
				.right = x + w > windowWidth ? windowWidth - x : w,
				.bottom = y + h > windowHeight ? windowHeight - y : h
			};
			blendFunction.SourceConstantAlpha = color >> 24;
			const HBRUSH clr = CreateSolidBrush(changeColorFormat(color));
			FillRect(assistDC, &rect, clr);
			deleteObject(clr);
			if (!AlphaBlend(canvasDC, x, y, rect.right, rect.bottom, assistDC, 0, 0, rect.right, rect.bottom, blendFunction)) Logger.warn(L"AlphaBlend failed");
		}
	}

	void fill(const RECT* const rect, const unsigned int color) const override {
		assertRendering();
		//assertRenderThread();
		if ((color & 0xff000000) == 0) return;
		if ((color & 0xff000000) == 0xff000000) {
			const HBRUSH clr = CreateSolidBrush(changeColorFormat(color));
			FillRect(canvasDC, rect, clr);
			deleteObject(clr);
		}
		else {
			const RECT r{
				.left = 0,
				.top = 0,
				.right = rect->right > windowWidth ? windowWidth : rect->right - rect->left,
				.bottom = rect->bottom > windowHeight ? windowHeight : rect->bottom - rect->top
			};
			blendFunction.SourceConstantAlpha = color >> 24;
			const HBRUSH clr = CreateSolidBrush(changeColorFormat(color));
			FillRect(assistDC, &r, clr);
			deleteObject(clr);
			if (!AlphaBlend(canvasDC, rect->left, rect->top, r.right, r.bottom, assistDC, 0, 0, r.right, r.bottom, blendFunction)) Logger.error(L"AlphaBlend failed");
		}
	}

	void fillWorld(const Vector2D& from, const Vector2D& to, const unsigned int color) const override {
		RECT rect{};
		Vector2D vector = (from - camera.getCurrentPosition()) * interactSettings.actual.mapScale;
		rect.left = static_cast<long>(vector.getX()) + (windowWidth >> 1);
		if (rect.left >= windowWidth) return;
		rect.top = static_cast<long>(vector.getY()) + (windowHeight >> 1);
		if (rect.top >= windowHeight) return;
		vector = (to - camera.getCurrentPosition()) * interactSettings.actual.mapScale;
		rect.right = static_cast<long>(vector.getX()) + (windowWidth >> 1);
		if (rect.right < 0) return;
		rect.bottom = static_cast<long>(vector.getY()) + (windowHeight >> 1);
		if (rect.bottom < 0) return;
		fill(&rect, color);
	}

	void fillWorld(const Vector2D& from, const double blockWidth, const double blockHeight, const unsigned int color) const override {
		RECT rect{};
		Vector2D vector = (from - camera.getCurrentPosition()) * interactSettings.actual.mapScale;
		rect.left = static_cast<long>(vector.getX()) + (windowWidth >> 1);
		if (rect.left >= windowWidth) return;
		rect.top = static_cast<long>(vector.getY()) + (windowHeight >> 1);
		if (rect.top >= windowHeight) return;
		vector += Vector2D(blockWidth, blockHeight).multiply(interactSettings.actual.mapScale);
		rect.right = static_cast<long>(vector.getX()) + (windowWidth >> 1);
		if (rect.right < 0) return;
		rect.bottom = static_cast<long>(vector.getY()) + (windowHeight >> 1);
		if (rect.bottom < 0) return;
		fill(&rect, color);
	}

	void fillWorldBlock(const BlockLocation& from, const unsigned int color) const override { fillWorld(from.getPosition(), 1, 1, color); }
	void texture(TextureEntry, int x, int y, int w, int h) const noexcept override {}

	void textureWorld(TextureEntry entry, Vector2D from, Vector2D targetSize, const unsigned char alpha = 255, const POINT srcFrom = {}, POINT srcSize = {}) const noexcept override {
		targetSize = world2client(from + targetSize);
		from = world2client(from);
		targetSize.subtract(from);
		const GdiTexture* const texture = assert_dynamic_cast<GdiTexture*>(textureOf(entry));
		SelectObject(assistDC, texture->bitmap);
		const ScopeGuard guard{[this] { SelectObject(assistDC, assistBitmap); }};
		if (srcSize.x <= 0 && srcSize.y <= 0) {
			srcSize.x = texture->width;
			srcSize.y = texture->height;
			if (texture->withSrcAlpha) {
				blendFunctionSrcAlpha.SourceConstantAlpha = alpha;
				AlphaBlend(canvasDC, static_cast<int>(from.getX()), static_cast<int>(from.getY()), static_cast<int>(targetSize.getX()), static_cast<int>(targetSize.getY()), assistDC, srcFrom.x, srcFrom.y, srcSize.x, srcSize.y, blendFunctionSrcAlpha);
			}
			else StretchBlt(canvasDC, static_cast<int>(from.getX()), static_cast<int>(from.getY()), static_cast<int>(targetSize.getX()), static_cast<int>(targetSize.getY()), assistDC, srcFrom.x, srcFrom.y, srcSize.x, srcSize.y, SRCCOPY);
		}
		else {
			BLENDFUNCTION& function = texture->withSrcAlpha ? blendFunctionSrcAlpha : blendFunction;
			function.SourceConstantAlpha = alpha;
			AlphaBlend(canvasDC, static_cast<int>(from.getX()), static_cast<int>(from.getY()), static_cast<int>(targetSize.getX()), static_cast<int>(targetSize.getY()), assistDC, srcFrom.x, srcFrom.y, srcSize.x, srcSize.y, function); // TODO(EmsiaetKadosh): need textureDC
		}
	}
};

extern IRenderer& renderer;
