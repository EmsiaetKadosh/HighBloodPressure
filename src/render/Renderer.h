//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#include "..\def.h"
#include "..\utils\math.h"
#include "..\utils\exception.h"
#include "..\utils\Task.h"
#include "..\interact\InteractManager.h"
#include "..\game\world\Location.h"
#include "..\utils\Chars.h"

class Game;

/**
 * 用于标记相对位置。
 */
enum class UILocation : char { LEFT_TOP, LEFT, LEFT_BOTTOM, TOP, CENTER, BOTTOM, RIGHT_TOP, RIGHT, RIGHT_BOTTOM };

interface IRenderable {
	virtual ~IRenderable() = default;
	virtual void render(double tickDelta) const noexcept = 0;
};

interface ITickable {
	virtual ~ITickable() = default;
	virtual void tick() noexcept = 0;
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
	Vector2D position; // 当前位置
	Vector2D velocity; // 移动速度，如果设置了平滑Camera
	Vector2D targetPosition; // Camera需要移动到的位置
	Entity* targeting = nullptr;

public:
	void setTargetEntity(Entity* target) noexcept { targeting = target; }
	[[nodiscard]] double getCurrentX() const noexcept { return position.getX(); }
	[[nodiscard]] double getCurrentY() const noexcept { return position.getY(); }
	[[nodiscard]] double getTargetX() const noexcept { return targetPosition.getX(); }
	[[nodiscard]] double getTargetY() const noexcept { return targetPosition.getY(); }
	[[nodiscard]] Vector2D getCurrentPosition() const noexcept { return position; }
	[[nodiscard]] Vector2D getVelocity() const noexcept { return velocity; }
	[[nodiscard]] Vector2D getTargetPosition() const noexcept { return targetPosition; }
};

class Renderer final : public ITickable {
	friend class Game;
	friend class Font;
	friend LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	inline static BLENDFUNCTION blendFunction = {
		.BlendOp = AC_SRC_OVER, // Only
		.BlendFlags = 0, // Must 0
		.SourceConstantAlpha = 255, // 预乘
		.AlphaFormat = 0, // Not AC_SRC_ALPHA
	};
	mutable List<HGDIOBJ> failed;
	mutable Camera camera;
	HDC MainDC = nullptr; // 8
	HDC resizeCopyDC = nullptr; // 8
	HBITMAP resizeCopyBitmap = nullptr; // 8
	HDC canvasDC = nullptr; // 8
	HBITMAP canvasBitmap = nullptr; // 8
	HDC assistDC = nullptr; // 8
	HBITMAP assistBitmap = nullptr; // 8
	std::thread::id renderThread = std::this_thread::get_id();
	int windowWidth = 0, windowHeight = 0; // 4 + 4
	/**
	 * 指示实时大小。为了防抖，只会在改变窗口大小结束时resize并重写windowWidth和windowHeight
	 */
	int syncWidth = 0, syncHeight = 0; // 4 + 4
	/**
	 * 缓存resizeCopyBitmap的宽高
	 */
	int resizeCopyWidth = 0, resizeCopyHeight = 0; // 4 + 4
	bool isRendering = false; // 1
	bool isResizing = false; // 1
	bool isResizeRequired = false; // 1
	char refreshedHDC = -1;

public:
	byte reserved[4]{}; // 4
	Task resizeReloadBitmap{nullptr};

private:
	void gameStartRender() noexcept;
	void gameEndRender() noexcept;
	/**
	 * 负责转发所有resize信息
	 */
	void resize(int width, int height) noexcept(false);

	static bool $deleteObject(const HGDIOBJ obj) noexcept {
		const unsigned type = GetObjectType(obj);
		if (!type) return true;
		switch (type) {
			case OBJ_BITMAP:
				Logger.info(L"DeleteObject failure: BITMAP");
				break;
			case OBJ_PEN:
				Logger.info(L"DeleteObject failure: PEN");
				break;
			case OBJ_BRUSH:
				Logger.info(L"DeleteObject failure: BRUSH");
				break;
			case OBJ_FONT:
				Logger.info(L"DeleteObject failure: FONT");
				break;
			case OBJ_REGION:
				Logger.info(L"DeleteObject failure: REGION");
				break;
			case OBJ_DC:
				Logger.info(L"DeleteObject failure: DC");
				break;
			case OBJ_MEMDC:
				Logger.info(L"DeleteObject failure: MEMDC");
				break;
			case OBJ_PAL:
				Logger.info(L"DeleteObject failure: PAL");
				break;
			default:
				Logger.info(L"DeleteObject failure: ? " + std::to_wstring(type));
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
				else { failed.push_back(*iter); }
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
	Renderer() {
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
	}

	~Renderer() override {
		Logger.put(L"Renderer destroyed");
		// finalize();
	}

	void finalize(bool isRenderThread) noexcept;
	void requireResize() noexcept { isResizeRequired = true; }
	void syncSize(int width, int height) noexcept(false);
	[[nodiscard]] int getWidth() const noexcept { return windowWidth; }
	[[nodiscard]] int getHeight() const noexcept { return windowHeight; }
	[[nodiscard]] int getSyncWidth() const noexcept { return syncWidth; }
	[[nodiscard]] int getSyncHeight() const noexcept { return syncHeight; }
	[[nodiscard]] bool checkResizing() const noexcept { return isResizing; }
	void tick() noexcept override {}
	/**
	 * @attention 会忽略A透明度值
	 * @param argb ARGB式颜色
	 * @return int BGR式颜色
	 */
	[[nodiscard]] static unsigned int changeColorFormat(const unsigned int argb) { return argb << 16 & 0xff0000 | argb & 0xff00 | argb >> 16 & 0xff; }
	void assertRendering() const noexcept(false) { if (!isRendering) throw InvalidOperationException(L"Operation should be done while rendering"); }
	void assertRenderThread() const noexcept(false) { if (std::this_thread::get_id() != renderThread) throw InvalidOperationException(L"Operation should be done in render thread"); }
	Camera& getCamera() const noexcept { return camera; }
	void resizeStart() noexcept { isResizing = true; }
	void resizeShow() const noexcept { StretchBlt(MainDC, 0, 0, syncWidth, syncHeight, resizeCopyDC, 0, 0, resizeCopyWidth, resizeCopyHeight, SRCCOPY); }
	void resizeEnd() noexcept;

	void fill(const int x, const int y, const int w, const int h, const unsigned int color) const {
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
			if (!AlphaBlend(canvasDC, x, y, rect.right, rect.bottom, assistDC, 0, 0, rect.right, rect.bottom, blendFunction)) Logger.error(L"AlphaBlend failed");
		}
	}

	void fill(const RECT* const rect, const unsigned int color) const {
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

	void fillWorld(const Vector2D& from, const Vector2D& to, const unsigned int color) const {
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

	void fillWorld(const Vector2D& from, const double blockWidth, const double blockHeight, const unsigned int color) const {
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

	void fillWorldBlock(const BlockLocation& from, const unsigned int color) const { fillWorld(from.getPosition(), 1, 1, color); }
};

extern Renderer renderer;
