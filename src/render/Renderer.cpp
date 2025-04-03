//
// Created by EmsiaetKadosh on 25-1-14.
//

#include "Renderer.h"

#include "..\game\Game.h"
#include "..\hbp.h"
#include "..\game\entity\Entity.h"
#include "..\interact\InteractManager.h"

void Camera::render(const double tickDelta, const QWORD tickRendering) noexcept {
	renderingTarget = targeting;
	if (renderingTarget) {
		renderingTarget->getMomentum().atomicAcquire();
		targetPosition = renderingTarget->getLocation(tickDelta, tickRendering).getPosition();
		renderingTarget->getMomentum().atomicRelease();
		Vector2D&& rel = targetPosition - position;
		if (dEquals(rel.lengthManhattan(), 0)) position = targetPosition;
		else position.add(rel.multiply(1 - interactSettings.constants.smoothCamera));
	}
}

void Camera::setTargetEntity(Entity* target) noexcept {
	updateTick = game.getTick();
	targeting = target;
}

void Renderer::gameStartRender() noexcept {
	isRendering = true;
	renderThread = std::this_thread::get_id();
	if (!MainDC || isResizeRequired || GetPixel(canvasDC, windowWidth - 1, windowHeight - 1) == 0xffffffff) resize(syncWidth, syncHeight);
	if (!MainDC) return;
	renderer.fill(0, interactSettings.actual.captionHeight, renderer.getWidth(), renderer.getHeight(), 0xff000000);
}

void Renderer::gameEndRender() noexcept {
	fontManager.get(1).draw(L"FPS: " + std::to_wstring(fps), 0, interactSettings.actual.captionHeight, 0xffee0000);
	fontManager.get(1).draw(L"TPS: " + std::to_wstring(tps), 0, interactSettings.actual.captionHeight + interactSettings.actual.fontHeight, 0xffee0000);
	isRendering = false;
	BitBlt(MainDC, 0, 0, windowWidth, windowHeight, canvasDC, 0, 0, SRCCOPY);
	if (isResizing) {
		if (!resizeCopyBitmap) {
			resizeCopyWidth = windowWidth;
			resizeCopyHeight = windowHeight;
			resizeCopyBitmap = CreateCompatibleBitmap(MainDC, windowWidth, windowHeight);
		} else if (resizeCopyWidth != windowWidth || resizeCopyHeight != windowHeight) {
			renderer.deleteObject(resizeCopyBitmap);
			resizeCopyBitmap = CreateCompatibleBitmap(MainDC, windowWidth, windowHeight);
		}
		SelectObject(resizeCopyDC, resizeCopyBitmap);
		BitBlt(resizeCopyDC, 0, 0, windowWidth, windowHeight, canvasDC, 0, 0, SRCCOPY);
	}
}

/**
 * 有一些事情必须在当前线程（renderThread）做，否则可能会有问题。
 * 虽然执行到此处程序必然是已经准备终止了，但是还是遵守一下。
 */
void Renderer::finalize(const bool isRenderThread) noexcept {
	if (isRenderThread) {
		if (assistDC) DeleteDC(assistDC), assistDC = nullptr;
		if (canvasDC) DeleteDC(canvasDC), canvasDC = nullptr;
		if (resizeCopyDC) DeleteDC(resizeCopyDC), resizeCopyDC = nullptr;
		if (canvasBitmap) DeleteObject(canvasBitmap), canvasBitmap = nullptr;
		if (assistBitmap) DeleteObject(assistBitmap), assistBitmap = nullptr;
		if (resizeCopyBitmap) DeleteObject(resizeCopyBitmap), resizeCopyBitmap = nullptr;
	} else if (MainDC) DeleteDC(MainDC), MainDC = nullptr;
}

void Renderer::resize(const int width, const int height) noexcept(false) {
	if (refreshedHDC == -1) { // 尚未Post请求
		lastPostRefreshTime = getCurrentTime();
		if (!PostMessageW(MainWindowHandle, WM_APP_REQUESTHDC, 0, 0)) {
			isRunning = false;
			Logger.error(L"PostMessage WM_APP_REQUESTHDC failed. LastError: " + std::to_wstring(GetLastError()));
		}
		Logger.debug(L"Resize post request");
		refreshedHDC = 0;
		return;
	}
	if (refreshedHDC == 0) {
		Logger.trace(L"Resize waiting for HDC");
		if (getCurrentTime() - lastPostRefreshTime >= std::chrono::seconds(1)) {
			Logger.debug(L"Resize re-post");
			if (!PostMessageW(MainWindowHandle, WM_APP_REQUESTHDC, 0, 0)) {
				isRunning = false;
				Logger.error(L"PostMessage WM_APP_REQUESTHDC failed. LastError: " + std::to_wstring(GetLastError()));
			} else lastPostRefreshTime = getCurrentTime();
		}
		return; // 已经Post，但是尚未获取到新的
	}
	// refreshedHDC == 1; 已经获取到新的
	refreshedHDC = -1;
	isResizeRequired = false;
	Logger.print(L"Resize:", width, height, L"Time:", getRunTime().count());
	const bool flag = windowWidth != width || windowHeight != height;
	windowWidth = width;
	windowHeight = height;
	deleteDC(canvasDC);
	deleteDC(assistDC);
	canvasDC = CreateCompatibleDC(MainDC);
	assistDC = CreateCompatibleDC(canvasDC);
	SetBkMode(canvasDC, TRANSPARENT);
	SetBkMode(assistDC, TRANSPARENT);
	deleteObject(canvasBitmap);
	deleteObject(assistBitmap);
	canvasBitmap = CreateCompatibleBitmap(MainDC, width, height);
	assistBitmap = CreateCompatibleBitmap(MainDC, width, height);
	SelectObject(canvasDC, canvasBitmap);
	SelectObject(assistDC, assistBitmap);
	if (!canvasBitmap || !assistBitmap) {
		if (!resizeReloadBitmap.getContainer()) {
			game.tasks.pushThis(resizeReloadBitmap);
			Logger.debug(L"Failed to create bitmap. Pushed task.");
		}
	} else Logger.debug(L"Successfully resized bitmap");
	if (flag) {
		interactSettings.setUiScale(static_cast<double>(height) / 2160.);
		game.handleResize();
		fontManager.resize(width, height);
	}
}

void Renderer::syncSize(const int width, const int height) noexcept(false) {
	syncWidth = width;
	syncHeight = height;
}

void Renderer::tick() noexcept(false) {
	mousePointingAtWorld = Vector2D(interactManager.getMouseX() - (windowWidth >> 1), interactManager.getMouseY() - (windowHeight >> 1)).divide(interactSettings.actual.mapScale).add(camera.getCurrentPosition());
	mousePointingAtBlock = mousePointingAtWorld;
}

void Renderer::resizeEnd() noexcept {
	isResizing = false;
	if (resizeCopyBitmap) deleteObject(resizeCopyBitmap);
	resizeCopyBitmap = nullptr;
	resizeCopyWidth = 0;
	resizeCopyHeight = 0;
	requireResize();
}

void Renderer::renderMouseWorld() noexcept {
	if (!interactManager.isInWindow()) return;
	fillWorldBlock(mousePointingAtBlock, mousePointingAtFlash.adaptsColor(0x88ffffff, 0x88ff0000));
	fillWorld(mousePointingAtWorld - Vector2D(10, 0.02), 20, 0.04, 0xffee0000);
	fillWorld(mousePointingAtWorld - Vector2D(0.02, 10), 0.04, 20, 0xffee0000);
}

inline Renderer renderer = Renderer();
