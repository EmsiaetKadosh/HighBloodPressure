#pragma once

#include "def.h"

inline HRESULT RemoveDefaultCaption(const HWND hWnd, const MARGINS* p) noexcept { return DwmExtendFrameIntoClientArea(hWnd, p); }

inline bool ShowConsoleIO() noexcept {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r+", stdin);
	return true;
}

inline const String ApplicationName = L"High Blood Pressure";
inline HINSTANCE MainInstance;
inline HWND MainWindowHandle;
inline Thread GameThread;
inline Thread RenderThread;
inline bool isRunning = true;
