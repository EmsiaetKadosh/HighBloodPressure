//
// Created by EmsiaetKadosh on 25-1-14.
//

#include "Renderer.h"

#include "..\game\Game.h"
#include "..\hbp.h"
#include "..\interact\InteractManager.h"

void Renderer::gameStartRender() noexcept {
	isRendering = true;
	renderThread = std::this_thread::get_id();
	renderer.fill(0, interactSettings.actual.captionHeight, renderer.getWidth(), renderer.getHeight(), 0xff000000);
}

void Renderer::gameEndRender() noexcept {
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

void Renderer::initialize() noexcept {
	if (MainDC && assistDC && canvasDC) return;
	MainDC = GetDC(MainWindowHandle);
	canvasDC = CreateCompatibleDC(MainDC);
	assistDC = CreateCompatibleDC(canvasDC);
	resizeCopyDC = CreateCompatibleDC(MainDC);
	canvasBitmap = CreateCompatibleBitmap(MainDC, windowWidth, windowHeight);
	SelectObject(canvasDC, canvasBitmap);
	assistBitmap = CreateCompatibleBitmap(canvasDC, windowWidth, windowHeight);
	SelectObject(assistDC, assistBitmap);
	if (!canvasDC) Logger.error(L"canvasDC is nullptr");
	if (!assistDC) Logger.error(L"assistDC is nullptr");
	if (!resizeCopyDC) Logger.error(L"resizeCopyDC is nullptr");
	if (!canvasBitmap) Logger.error(L"canvasBitmap is nullptr");
	if (!assistBitmap) Logger.error(L"assistBitmap is nullptr");
	SetBkMode(MainDC, TRANSPARENT);
	SetBkMode(canvasDC, TRANSPARENT);
	SetBkMode(assistDC, TRANSPARENT);
	RECT rect;
	GetWindowRect(MainWindowHandle, &rect);
	// resize(rect.right - rect.left, rect.bottom - rect.top);
}

/**
 * 有一些事情必须在当前线程（renderThread）做，否则可能会有问题。
 * 虽然执行到此处程序必然是已经准备终止了，但是还是遵守一下。
 */
void Renderer::finalize() noexcept {
	if (MainDC) DeleteDC(MainDC), MainDC = nullptr;
	if (assistDC) DeleteDC(assistDC), assistDC = nullptr;
	if (canvasDC) DeleteDC(canvasDC), canvasDC = nullptr;
	if (resizeCopyDC) DeleteDC(resizeCopyDC), resizeCopyDC = nullptr;
	if (canvasBitmap) DeleteObject(canvasBitmap), canvasBitmap = nullptr;
	if (assistBitmap) DeleteObject(assistBitmap), assistBitmap = nullptr;
	if (resizeCopyBitmap) DeleteObject(resizeCopyBitmap), resizeCopyBitmap = nullptr;
}

void Renderer::resize(const int width, const int height) noexcept(false) {
	const bool flag = windowWidth != width || windowHeight != height;
	windowWidth = width;
	windowHeight = height;
	std::atomic_thread_fence(std::memory_order_seq_cst);
	const HBITMAP canvas = canvasBitmap, assist = assistBitmap;
	canvasBitmap = CreateCompatibleBitmap(MainDC, width, height);
	assistBitmap = CreateCompatibleBitmap(assistDC, width, height);
	SelectObject(canvasDC, canvasBitmap);
	SelectObject(assistDC, assistBitmap);
	deleteObject(canvas);
	deleteObject(assist);
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

inline Renderer renderer = Renderer();
