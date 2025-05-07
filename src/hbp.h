#pragma once

#include "def.h"
#include "utils\exception.h"

inline HRESULT RemoveDefaultCaption(const HWND hWnd, const MARGINS* p) noexcept { return DwmExtendFrameIntoClientArea(hWnd, p); }

inline const String ApplicationName = L"High Blood Pressure";
inline HINSTANCE MainInstance;
inline HWND MainWindowHandle;
inline Thread GameThread;
inline Thread RenderThread;
inline bool isRunning = true;

inline void ShowConsoleIO() noexcept {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}
