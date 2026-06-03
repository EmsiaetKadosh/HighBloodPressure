
#pragma once

#include "Windows.h"

#define WM_APP_LBUTTONUP (WM_APP + 1)
#define WM_APP_MBUTTONDOWN (WM_APP + 2)
#define WM_APP_GAMESTART (WM_APP + 3)
#define WM_APP_EXITSIZEMOVE (WM_APP + 4)
#define WM_APP_REQUESTHDC (WM_APP + 5)
#define WM_APP_TERMINATE (WM_APP + 6)

inline HWND MainWindowHandle = nullptr;
inline HINSTANCE MainInstance = nullptr;
