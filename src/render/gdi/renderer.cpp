//
// Created by EmsiaetKadosh on 25-6-18.
//

#pragma message("using GDI file " __FILE__)

#include "..\..\utils\IText.h"
#include "..\..\game\Game.h"
#include "renderer.hpp"
#include "texture.hpp"
#include "font.hpp"

void GdiRenderer::gameStartRender() noexcept {
	isRendering = true;
	renderThread = std::this_thread::get_id();
	if (!MainDC || isResizeRequired || GetPixel(canvasDC, windowWidth - 1, windowHeight - 1) == 0xffffffff) resize(syncWidth, syncHeight);
	if (!MainDC) return;
	fill(0, interactSettings.actual.captionHeight, getWidth(), getHeight(), 0xff000000);
}

void GdiRenderer::gameEndRender() noexcept {
	fontManager.get(2).draw(L"FPS: " + std::to_wstring(fps), 0, interactSettings.actual.captionHeight, 0xffee0000);
	fontManager.get(2).draw(L"TPS: " + std::to_wstring(tps), 0, interactSettings.actual.captionHeight + interactSettings.actual.fontHeight, 0xffee0000);
	isRendering = false;
	BitBlt(MainDC, 0, 0, windowWidth, windowHeight, canvasDC, 0, 0, SRCCOPY);
	if (isResizing) {
		if (!resizeCopyBitmap) {
			resizeCopyWidth = windowWidth;
			resizeCopyHeight = windowHeight;
			resizeCopyBitmap = CreateCompatibleBitmap(MainDC, windowWidth, windowHeight);
		} else if (resizeCopyWidth != windowWidth || resizeCopyHeight != windowHeight) {
			deleteObject(resizeCopyBitmap);
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
void GdiRenderer::finalize(const bool isRenderThread) noexcept {
	fontManager.finalize();
	if (isRenderThread) {
		if (assistDC) DeleteDC(assistDC), assistDC = nullptr;
		if (canvasDC) DeleteDC(canvasDC), canvasDC = nullptr;
		if (resizeCopyDC) DeleteDC(resizeCopyDC), resizeCopyDC = nullptr;
		if (canvasBitmap) DeleteObject(canvasBitmap), canvasBitmap = nullptr;
		if (assistBitmap) DeleteObject(assistBitmap), assistBitmap = nullptr;
		if (resizeCopyBitmap) DeleteObject(resizeCopyBitmap), resizeCopyBitmap = nullptr;
		textureManager.unload();
	} else if (MainDC) DeleteDC(MainDC), MainDC = nullptr;
}

void GdiRenderer::resize(const int width, const int height) noexcept(false) {
	Logger.info(L"resize called");
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
	if (!canvasDC) canvasDC = CreateCompatibleDC(MainDC);
	if (!assistDC) assistDC = CreateCompatibleDC(canvasDC);
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
		setSystemScale();
		game.handleResize();
		fontManager.resize(width, height);
	}
}

IRenderer& GdiRenderer::postInitialize() noexcept { return *this; }

void GdiRenderer::tick() noexcept(false) {}
void GdiRenderer::textureWorld(TextureEntry entry, Vector2D from, Vector2D targetSize, const unsigned char alpha, const POINT srcFrom, POINT srcSize) const noexcept {
	targetSize = world2client(from + targetSize);
	from = world2client(from);
	targetSize.subtract(from);
	const GdiTexture* const texture = assert_dynamic_cast<GdiTexture*>(textureOf(entry));
	if (texture->bitmap) SelectObject(assistDC, texture->bitmap);
	else {
		TextureEntry entry = textureManager.getNullTexture();
		SelectObject(assistDC, assert_dynamic_cast<GdiTexture*>(textureOf(entry))->bitmap);
	}
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

void GdiRenderer::resizeEnd() noexcept {
	isResizing = false;
	if (resizeCopyBitmap) deleteObject(resizeCopyBitmap);
	resizeCopyBitmap = nullptr;
	resizeCopyWidth = 0;
	resizeCopyHeight = 0;
	requireResize();
}

void GdiRenderer::renderMouseWorld() noexcept {
	mousePointingAtWorld = client2world(interactManager.getMouseX(), interactManager.getMouseY());
	mousePointingAtBlock = mousePointingAtWorld;
	if (!interactManager.isInWindow()) return;
	fillWorldBlock(mousePointingAtBlock, mousePointingAtFlash.adaptsColor(0x88ffffff, 0x88ff0000));
	fillWorld(mousePointingAtWorld - Vector2D(10, 0.02), 20, 0.04, 0xffee0000);
	fillWorld(mousePointingAtWorld - Vector2D(0.02, 10), 0.04, 20, 0xffee0000);
}
