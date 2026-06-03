
#include <Windows.h>
#include <iostream>
#include "src\main.hpp"
#include "src\windows.hpp"

#define SHORT_CODE_TEST false

void shortTest() {
	const Vector3D
		up = { 0, 1, 0 },
		forward = { 0, 0, 1 };
	Logger.of(L"up * forward = ", up.cross(forward).toString()).debug();
	Logger.of(
		L"Context example:\n",
		DirectTextureContext()
		.at({ 0, 0, 1 }, 0xffee0000, { 1.0f, 0.5f })
		.vertices
		.front()
		.toString()
	).debug();
}

inline int MessageLoop() noexcept;
void ProcessRawInput(HRAWINPUT, bool);
LRESULT __stdcall WindowCallback(HWND, UINT, WPARAM, LPARAM);
LRESULT __stdcall HookCallback(int, WPARAM, LPARAM);
inline bool SystemInitialize(WNDCLASSEX&, HHOOK&, RAWINPUTDEVICE&, int);
inline void SystemFinalize(const WNDCLASSEX&, const HHOOK&, RAWINPUTDEVICE&);

int __stdcall wWinMain(const HINSTANCE hInstance, const HINSTANCE, [[maybe_unused]] const LPWSTR lpCmdLine, [[maybe_unused]] const int nShowCmd) {
	MainInstance = hInstance;
	WNDCLASSEX wc = {};
	HHOOK hook = nullptr;
	RAWINPUTDEVICE device = {};
	int ret = 0;
	if (SystemInitialize(wc, hook, device, nShowCmd)) {
		MessageBoxW(nullptr, L"系统初始化失败", ApplicationName.c_str(), MB_OK);
		return FALSE;
	}
	shortTest();
	if constexpr (!SHORT_CODE_TEST) {
		game.initialize();
		ret = MessageLoop();
		game.finalize();
	}
	if constexpr (!SHORT_CODE_TEST) SystemFinalize(wc, hook, device);
	system("pause");
	return ret;
}

int MessageLoop() noexcept {
	MSG msg = { nullptr };
	const HACCEL hAccelTable = LoadAcceleratorsW(MainInstance, MAKEINTRESOURCE(109));
	while (GetMessageW(&msg, nullptr, 0, 0))
		if (!TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	DestroyAcceleratorTable(hAccelTable);
	return static_cast<int>(msg.wParam);
}

void ProcessRawInput(const HRAWINPUT handle, const bool front) {
	RAWINPUT rawInput = {};
	static unsigned int sizeofRawInput = sizeof(RAWINPUT);
	if (!GetRawInputData(handle, RID_INPUT, &rawInput, &sizeofRawInput, sizeof(RAWINPUTHEADER))) return Logger.error(L"Failed to get header"), void();
	const bool keyDown = !(rawInput.data.keyboard.Flags & RI_KEY_BREAK);
	if (keyDown && !front) return;
	if (rawInput.data.keyboard.VKey == VK_SHIFT) {
		// FIXME: temp code
		// TODO(EmsiaetKadosh): 目前没有什么很好的替代方案来区分LSHIFT和RSHIFT，只能通过KeyCode来区分
		if (rawInput.data.keyboard.MakeCode == 0x2A) return game.getInteractManager().update(VK_LSHIFT, keyDown);
		if (rawInput.data.keyboard.MakeCode == 0x36) return game.getInteractManager().update(VK_RSHIFT, keyDown);
		return game.getInteractManager().update(VK_SHIFT, keyDown);
	}
	if (rawInput.data.keyboard.VKey == VK_CONTROL) {
		if (rawInput.data.keyboard.Flags & RI_KEY_E0) game.getInteractManager().update(VK_RCONTROL, keyDown);
		else game.getInteractManager().update(VK_LCONTROL, keyDown);
	}
	game.getInteractManager().update(static_cast<unsigned char>(rawInput.data.keyboard.VKey), keyDown);
}

[[nodiscard]] inline bool SystemInitialize(WNDCLASSEX& wc, HHOOK& hook, RAWINPUTDEVICE& device, const int nShowCmd) {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	SetConsoleOutputCP(65001);
	SetUnhandledExceptionFilter(UnhandledExceptionFilter);

	if constexpr (SHORT_CODE_TEST) return false; // 不执行窗口创建

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowCallback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = MainInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = L"None";
	wc.lpszClassName = ApplicationName.c_str();
	if (!RegisterClassExW(&wc)) return true;
	if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) Logger.of(L"SetProcessDpiAwarenessContext failed. LastError:", GetLastError()).log();
	MainWindowHandle = CreateWindowExW(0, wc.lpszClassName, wc.lpszClassName, WS_VISIBLE | WS_MAXIMIZEBOX | WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, MainInstance, nullptr);
#if false // 移除原有标题栏
	constexpr MARGINS margins = {};
	UNREFERENCED_PARAMETER(RemoveDefaultCaption(MainWindowHandle, &margins));
#endif
	// SetWindowLongW(MainWindowHandle, GWL_EXSTYLE, GetWindowLongW(MainWindowHandle, GWL_EXSTYLE) | WS_EX_LAYERED);
	// SetLayeredWindowAttributes(MainWindowHandle, 0xffffff, 0xe0, LWA_COLORKEY /* | LWA_ALPHA */);
	SetWindowLongW(MainWindowHandle, GWL_STYLE, WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CAPTION);
	SetWindowPos(MainWindowHandle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	ShowWindow(MainWindowHandle, nShowCmd);
	// Hook
	hook = SetWindowsHookExW(WH_GETMESSAGE, HookCallback, nullptr, GetCurrentThreadId());
	if (!hook) Logger.of(L"SetWindowsHookW failed. LastError:", GetLastError()).error();
	// RawInput
	device.usUsagePage = 0x01;
	device.usUsage = 0x06;
	device.dwFlags = RIDEV_INPUTSINK; // 如果指定RIDEV_NOLEGACY则会禁用传统键盘消息
	device.hwndTarget = MainWindowHandle;
	if (!RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE))) Logger.of(L"RegisterRawInputDevice failed. LastError:", GetLastError()).error();
	else game.getInteractManager().enableRawInput();
	return false;
}

inline void SystemFinalize(const WNDCLASSEX& wc, const HHOOK& hook, RAWINPUTDEVICE& device) {
	if constexpr (SHORT_CODE_TEST) return; // 不执行手动解构
	if (!UnhookWindowsHookEx(hook)) Logger.error(L"Failed to UnhookWindowsHookEx. LastError:" + std::to_wstring(GetLastError()));
	if (!UnregisterClassW(wc.lpszClassName, wc.hInstance)) Logger.error(L"Failed to UnregisterClassW. LastError:" + std::to_wstring(GetLastError()));
	if (!RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE))) Logger.of(L"Failed to UnregisterRawInputDevice. LastError:", GetLastError()).error();
	device.dwFlags = RIDEV_REMOVE;
}

LRESULT __stdcall HookCallback(const int code, const WPARAM wParam, const LPARAM lParam) {
	if (code < 0) {
		Logger.log(L"HookProc nCode < 0");
		return CallNextHookEx(nullptr, code, wParam, lParam);
	}
	switch (const MSG* p = reinterpret_cast<MSG*>(lParam); p->message) {
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP:
			PostMessageW(MainWindowHandle, WM_APP_LBUTTONUP, p->wParam, p->lParam);
			return 0;
		case WM_MBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			PostMessageW(MainWindowHandle, WM_APP_MBUTTONDOWN, p->wParam, p->lParam);
			return 0;
		default:
			break;
	}
	return CallNextHookEx(nullptr, code, wParam, lParam);
}

LRESULT __stdcall WindowCallback(const HWND hwnd, const UINT msg, const WPARAM w, const LPARAM l) {
	switch (msg) {
		case WM_MOUSEMOVE: {
			RECT clientRect;
			GetClientRect(hwnd, &clientRect);
			POINT pt = {
				clientRect.right - clientRect.left >> 1,
				clientRect.bottom - clientRect.top >> 1
			};
			POINT ptCurrent;
			GetCursorPos(&ptCurrent); // 获取屏幕坐标
			ScreenToClient(hwnd, &ptCurrent); // 转换为客户区坐标
			game.getCamera().rotate((pt.y - ptCurrent.y) / 600.0, (pt.x - ptCurrent.x) / 600.0);
			ClientToScreen(hwnd, &pt);
			SetCursorPos(pt.x, pt.y);
		}
			return 0;
		case WM_KEYDOWN:
			game.getInteractManager().keydown(static_cast<int>(w));
			return 0;
		case WM_KEYUP:
			game.getInteractManager().keyup(static_cast<int>(w));
			return 0;
		case WM_SIZE: {
			// 从 lParam 直接提取宽高（这是最推荐的方法）
			const int newWidth = LOWORD(l); // 新宽度的低16位
			const int newHeight = HIWORD(l); // 新高度的高16位

			// wParam 参数表示调整大小的类型
			if constexpr (false)
				switch (w) {
					case SIZE_RESTORED: // 窗口被恢复（正常大小）
					case SIZE_MAXIMIZED: // 窗口被最大化
					case SIZE_MINIMIZED: // 窗口被最小化（此时宽高可能为0）
						break;
				}

			if (newWidth > 0 && newHeight > 0) game.getRenderer().setViewport(newWidth, newHeight);
			break;
		}
		case WM_APP_TERMINATE:
			Logger.ofNoexcept(L"Terminating").debug();
			DestroyWindow(MainWindowHandle);
			return 0;
		case WM_DESTROY:
			Logger.ofNoexcept(L"Destroying window").debug();
			PostQuitMessage(0);
			return 0;
		default:
			break;
	}

	return DefWindowProcW(hwnd, msg, w, l);
}
