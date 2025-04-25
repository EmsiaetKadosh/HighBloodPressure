
#include "includes.h"
#include "utils\TestCode.h"

long __stdcall UnhandledExceptionHandler(PEXCEPTION_POINTERS exception) {
	isRunning = false;
	Logger.error(L"Unhandled exception handler called");
	auto* const ContextRecord = exception->ContextRecord;
	auto* const ExceptionRecord = exception->ExceptionRecord;
	Logger.print(
		std::hex,
		L"Register parameter home addresses:"
		L"\n        P1Home", ContextRecord->P1Home,
		L"\n        P2Home", ContextRecord->P2Home,
		L"\n        P3Home", ContextRecord->P3Home,
		L"\n        P4Home", ContextRecord->P4Home,
		L"\n        P5Home", ContextRecord->P5Home,
		L"\n        P6Home", ContextRecord->P6Home);
	Logger.print(
		L"Control Flags:"
		L"\n  ContextFlags", ContextRecord->ContextFlags,
		L"\n         MxCsr", ContextRecord->MxCsr);
	Logger.print(
		L"Segment Registers and processor flags:"
		L"\n         SegCs", ContextRecord->SegCs,
		L"\n         SegDs", ContextRecord->SegDs,
		L"\n         SegEs", ContextRecord->SegEs,
		L"\n         SegFs", ContextRecord->SegFs,
		L"\n         SegGs", ContextRecord->SegGs,
		L"\n         SegSs", ContextRecord->SegSs,
		L"\n        EFlags", ContextRecord->EFlags);
	Logger.print(
		L"Debug registers:"
		L"\n            Dr0", ContextRecord->Dr0,
		L"\n            Dr1", ContextRecord->Dr1,
		L"\n            Dr2", ContextRecord->Dr2,
		L"\n            Dr3", ContextRecord->Dr3,
		L"\n            Dr6", ContextRecord->Dr6,
		L"\n            Dr7", ContextRecord->Dr7);
	Logger.print(
		L"Integer registers:"
		L"\n             Rax", ContextRecord->Rax,
		L"\n             Rcx", ContextRecord->Rcx,
		L"\n             Rdx", ContextRecord->Rdx,
		L"\n             Rbx", ContextRecord->Rbx,
		L"\n             Rsp", ContextRecord->Rsp,
		L"\n             Rbp", ContextRecord->Rbp,
		L"\n             Rsi", ContextRecord->Rsi,
		L"\n             Rdi", ContextRecord->Rdi,
		L"\n              R8", ContextRecord->R8,
		L"\n              R9", ContextRecord->R9,
		L"\n             R10", ContextRecord->R10,
		L"\n             R11", ContextRecord->R11,
		L"\n             R12", ContextRecord->R12,
		L"\n             R13", ContextRecord->R13,
		L"\n             R14", ContextRecord->R14,
		L"\n             R15", ContextRecord->R15);
	Logger.print(
		L"Program Counter:"
		L"\n             Rip", ContextRecord->Rip);
	Logger.print(
		L"Floating point state:"
		L"\n  XMM_SAVE_AREA32:"
		L"\n          ControlWord", ContextRecord->FltSave.ControlWord,
		L"\n           StatusWord", ContextRecord->FltSave.StatusWord,
		L"\n              TagWord", ContextRecord->FltSave.TagWord,
		L"\n            Reserved1", ContextRecord->FltSave.Reserved1,
		L"\n          ErrorOpcode", ContextRecord->FltSave.ErrorOpcode,
		L"\n          ErrorOffset", ContextRecord->FltSave.ErrorOffset,
		L"\n        ErrorSelector", ContextRecord->FltSave.ErrorSelector,
		L"\n            Reserved2", ContextRecord->FltSave.Reserved2,
		L"\n           DataOffset", ContextRecord->FltSave.DataOffset,
		L"\n         DataSelector", ContextRecord->FltSave.DataSelector,
		L"\n            Reserved3", ContextRecord->FltSave.Reserved3,
		L"\n                MxCsr", ContextRecord->FltSave.MxCsr,
		L"\n           MxCsr_Mask", ContextRecord->FltSave.MxCsr_Mask);
	Logger.print(
		L"\n    FloatRegisters[0]",
		ContextRecord->FltSave.FloatRegisters[0].High,
		ContextRecord->FltSave.FloatRegisters[0].Low,
		L"\n    FloatRegisters[1]",
		ContextRecord->FltSave.FloatRegisters[1].High,
		ContextRecord->FltSave.FloatRegisters[1].Low,
		L"\n    FloatRegisters[2]",
		ContextRecord->FltSave.FloatRegisters[2].High,
		ContextRecord->FltSave.FloatRegisters[2].Low,
		L"\n    FloatRegisters[3]",
		ContextRecord->FltSave.FloatRegisters[3].High,
		ContextRecord->FltSave.FloatRegisters[3].Low,
		L"\n    FloatRegisters[4]",
		ContextRecord->FltSave.FloatRegisters[4].High,
		ContextRecord->FltSave.FloatRegisters[4].Low,
		L"\n    FloatRegisters[5]",
		ContextRecord->FltSave.FloatRegisters[5].High,
		ContextRecord->FltSave.FloatRegisters[5].Low,
		L"\n    FloatRegisters[6]",
		ContextRecord->FltSave.FloatRegisters[6].High,
		ContextRecord->FltSave.FloatRegisters[6].Low,
		L"\n    FloatRegisters[7]",
		ContextRecord->FltSave.FloatRegisters[7].High,
		ContextRecord->FltSave.FloatRegisters[7].Low);
	Logger.print(
		L"\n       XmmRegisters[]"
		L"\n           Xmm0",
		ContextRecord->Xmm0.High,
		ContextRecord->Xmm0.Low,
		L"\n           Xmm1",
		ContextRecord->Xmm1.High,
		ContextRecord->Xmm1.Low,
		L"\n           Xmm2",
		ContextRecord->Xmm2.High,
		ContextRecord->Xmm2.Low,
		L"\n           Xmm3",
		ContextRecord->Xmm3.High,
		ContextRecord->Xmm3.Low,
		L"\n           Xmm4",
		ContextRecord->Xmm4.High,
		ContextRecord->Xmm4.Low,
		L"\n           Xmm5",
		ContextRecord->Xmm5.High,
		ContextRecord->Xmm5.Low,
		L"\n           Xmm6",
		ContextRecord->Xmm6.High,
		ContextRecord->Xmm6.Low,
		L"\n           Xmm7",
		ContextRecord->Xmm7.High,
		ContextRecord->Xmm7.Low,
		L"\n           Xmm8",
		ContextRecord->Xmm8.High,
		ContextRecord->Xmm8.Low,
		L"\n           Xmm9",
		ContextRecord->Xmm9.High,
		ContextRecord->Xmm9.Low,
		L"\n          Xmm10",
		ContextRecord->Xmm10.High,
		ContextRecord->Xmm10.Low,
		L"\n          Xmm11",
		ContextRecord->Xmm11.High,
		ContextRecord->Xmm11.Low,
		L"\n          Xmm12",
		ContextRecord->Xmm12.High,
		ContextRecord->Xmm12.Low,
		L"\n          Xmm13",
		ContextRecord->Xmm13.High,
		ContextRecord->Xmm13.Low,
		L"\n          Xmm14",
		ContextRecord->Xmm14.High,
		ContextRecord->Xmm14.Low,
		L"\n          Xmm15",
		ContextRecord->Xmm15.High,
		ContextRecord->Xmm15.Low);

	Logger.print(
		"\nVectorRegister"
		L"\n              0",
		ContextRecord->VectorRegister[0].High,
		ContextRecord->VectorRegister[0].Low,
		"\n               1",
		ContextRecord->VectorRegister[1].High,
		ContextRecord->VectorRegister[1].Low,
		L"\n               2",
		ContextRecord->VectorRegister[2].High,
		ContextRecord->VectorRegister[2].Low,
		L"\n               3",
		ContextRecord->VectorRegister[3].High,
		ContextRecord->VectorRegister[3].Low,
		L"\n               4",
		ContextRecord->VectorRegister[4].High,
		ContextRecord->VectorRegister[4].Low,
		L"\n               5",
		ContextRecord->VectorRegister[5].High,
		ContextRecord->VectorRegister[5].Low,
		L"\n               6",
		ContextRecord->VectorRegister[6].High,
		ContextRecord->VectorRegister[6].Low,
		L"\n               7",
		ContextRecord->VectorRegister[7].High,
		ContextRecord->VectorRegister[7].Low,
		L"\n               8",
		ContextRecord->VectorRegister[8].High,
		ContextRecord->VectorRegister[8].Low,
		L"\n               9",
		ContextRecord->VectorRegister[9].High,
		ContextRecord->VectorRegister[9].Low,
		L"\n              10",
		ContextRecord->VectorRegister[10].High,
		ContextRecord->VectorRegister[10].Low,
		L"\n              11",
		ContextRecord->VectorRegister[11].High,
		ContextRecord->VectorRegister[11].Low,
		L"\n              12",
		ContextRecord->VectorRegister[12].High,
		ContextRecord->VectorRegister[12].Low,
		L"\n              13",
		ContextRecord->VectorRegister[13].High,
		ContextRecord->VectorRegister[13].Low,
		L"\n              14",
		ContextRecord->VectorRegister[14].High,
		ContextRecord->VectorRegister[14].Low,
		L"\n              15",
		ContextRecord->VectorRegister[15].High,
		ContextRecord->VectorRegister[15].Low,
		L"\n              16",
		ContextRecord->VectorRegister[16].High,
		ContextRecord->VectorRegister[16].Low,
		L"\n              17",
		ContextRecord->VectorRegister[17].High,
		ContextRecord->VectorRegister[17].Low,
		L"\n              18",
		ContextRecord->VectorRegister[18].High,
		ContextRecord->VectorRegister[18].Low,
		L"\n              19",
		ContextRecord->VectorRegister[19].High,
		ContextRecord->VectorRegister[19].Low,
		L"\n              20",
		ContextRecord->VectorRegister[20].High,
		ContextRecord->VectorRegister[20].Low,
		L"\n              21",
		ContextRecord->VectorRegister[21].High,
		ContextRecord->VectorRegister[21].Low,
		L"\n              22",
		ContextRecord->VectorRegister[22].High,
		ContextRecord->VectorRegister[22].Low,
		L"\n              23",
		ContextRecord->VectorRegister[23].High,
		ContextRecord->VectorRegister[23].Low,
		L"\n              24",
		ContextRecord->VectorRegister[24].High,
		ContextRecord->VectorRegister[24].Low,
		L"\n              25",
		ContextRecord->VectorRegister[25].High,
		ContextRecord->VectorRegister[25].Low,
		L"\n   VectorControl", ContextRecord->VectorControl);
	Logger.print(
		L"\nSpecial debug control registers:"
		L"\n         DebugControl", ContextRecord->DebugControl,
		L"\n      LastBranchToRip", ContextRecord->LastBranchToRip,
		L"\n    LastBranchFromRip", ContextRecord->LastBranchFromRip,
		L"\n   LastExceptionToRip", ContextRecord->LastExceptionToRip,
		L"\n LastExceptionFromRip", ContextRecord->LastExceptionFromRip
		);

	Logger.print(
		L"\nExceptionRecord:"
		L"\n         ExceptionCode", ExceptionRecord->ExceptionCode,
		L"\n        ExceptionFlags", ExceptionRecord->ExceptionFlags,
		L"\n      ExceptionAddress", ExceptionRecord->ExceptionAddress,
		L"\n      NumberParameters", ExceptionRecord->NumberParameters,
		L"\n  ExceptionInformation",
		L"\n      ", ExceptionRecord->ExceptionInformation[0],
		L"\n      ", ExceptionRecord->ExceptionInformation[1],
		L"\n      ", ExceptionRecord->ExceptionInformation[2],
		L"\n      ", ExceptionRecord->ExceptionInformation[3],
		L"\n      ", ExceptionRecord->ExceptionInformation[4],
		L"\n      ", ExceptionRecord->ExceptionInformation[5],
		L"\n      ", ExceptionRecord->ExceptionInformation[6],
		L"\n      ", ExceptionRecord->ExceptionInformation[7],
		L"\n      ", ExceptionRecord->ExceptionInformation[8],
		L"\n      ", ExceptionRecord->ExceptionInformation[9],
		L"\n      ", ExceptionRecord->ExceptionInformation[10],
		L"\n      ", ExceptionRecord->ExceptionInformation[11],
		L"\n      ", ExceptionRecord->ExceptionInformation[12],
		L"\n      ", ExceptionRecord->ExceptionInformation[13],
		L"\n      ", ExceptionRecord->ExceptionInformation[14]
		);
	_wsystem(L"pause");
	return EXCEPTION_EXECUTE_HANDLER;
}

Time lastTick = getCurrentTime();

void gameThread() {
	try {
		Time lastTps = getCurrentTime();
		QWORD nanoSeconds = 0;
		unsigned int tickCount = 0;
		while (isRunning) {
			const Time thisTime = getCurrentTime();
			if (thisTime - lastTick < std::chrono::milliseconds(interactSettings.constants.msPerTick)) {
				Sleep(1);
				continue;
			}
			if (thisTime - lastTps >= std::chrono::seconds(1)) {
				renderer.tps = static_cast<double>(tickCount) / static_cast<double>((thisTime - lastTps).count());
				Logger.trace(std::to_wstring(tickCount / 10'000'000) + L" ticks cost " + std::to_wstring(nanoSeconds / 10000) + L" ms");
				nanoSeconds = 0;
				tickCount = 0;
				lastTps = thisTime;
			}
			tickCount += 10'000'000;
			game.currentTickFlag.atomicAcquire();
			std::atomic_thread_fence(std::memory_order_acquire);
			lastTick = thisTime;
			++game.currentTick;
			std::atomic_thread_fence(std::memory_order_acquire);
			game.currentTickFlag.atomicRelease();
			game.tick();
			nanoSeconds += (getCurrentTime() - thisTime).count();
		}
	} catch (const Exception& e) {
		Logger.error(L"Game thread exception: " + e.getMessage());
		printStackTrace(e.getStackTrace(), -10);
	}
	catch (const std::exception& e) { Logger.error(L"Game thread exception (builtin): " + atow(e.what())); }
	Logger.error(L"Game thread ended.");
	isRunning = false;
	DestroyWindow(MainWindowHandle);
}

void renderThread() {
	try {
		{
			RECT clientRect;
			GetClientRect(MainWindowHandle, &clientRect);
			renderer.syncSize(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
			renderer.requireResize();
		}
		Time lastRender = getCurrentTime(), lastFps = lastRender;
		QWORD nanoSeconds = 0;
		unsigned int frameCount = 0;
		while (isRunning) {
			const Time thisTime = getCurrentTime();
			if (thisTime - lastRender < std::chrono::milliseconds(interactSettings.constants.msPerRender)) {
				Sleep(1);
				continue;
			}
			if (thisTime - lastFps >= std::chrono::seconds(1)) {
				renderer.fps = static_cast<double>(frameCount) / static_cast<double>((thisTime - lastFps).count());
				Logger.trace(std::to_wstring(frameCount / 10'000'000) + L" frames cost " + std::to_wstring(nanoSeconds / 10000) + L" ms");
				nanoSeconds = 0;
				frameCount = 0;
				lastFps = thisTime;
			}
			frameCount += 10'000'000;
			game.currentTickFlag.atomicAcquire();
			std::atomic_thread_fence(std::memory_order_acquire);
			const Time lastTickFetch = lastTick;
			const QWORD tickRendering = game.currentTick;
			std::atomic_thread_fence(std::memory_order_acquire);
			game.currentTickFlag.atomicRelease();
			game.render(nRange(static_cast<double>((thisTime - lastTickFetch).count()) * 0.000'1 / static_cast<double>(interactSettings.constants.msPerTick), 0.0, 1.0), tickRendering);
			lastRender = thisTime;
			nanoSeconds += (getCurrentTime() - thisTime).count();
		}
	} catch (const Exception& e) {
		Logger.log(L"Render thread exception: " + e.getMessage());
		printStackTrace(e.getStackTrace(), -10);
	}
	catch (const std::exception& e) { Logger.log(L"Render thread exception (builtin): " + atow(e.what())); }
	Logger.error(L"Render thread ended.");
	isRunning = false;
	DestroyWindow(MainWindowHandle);
	renderer.finalize(true);
}

LRESULT __stdcall WndProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
	switch (uMsg) {
			[[likely]]
		case WM_PAINT: return Logger.trace(L"WM_PAINT"), ValidateRect(hwnd, nullptr), 0;
			[[likely]]
		case WM_NCHITTEST: {
			POINT point = { GET_X_LPARAM(lParam), (GET_Y_LPARAM(lParam)) };
			ScreenToClient(hwnd, &point);
			const int xPos = point.x;
			const int yPos = point.y;
			const bool zoomed = !IsZoomed(hwnd);
			if (zoomed) {
				if (xPos < interactSettings.actual.marginWidth) {
					if (yPos < interactSettings.actual.marginWidth) return HTTOPLEFT;
					if (renderer.getSyncHeight() - yPos < interactSettings.actual.marginWidth) return HTBOTTOMLEFT;
					return HTLEFT;
				}
				if (renderer.getSyncWidth() - xPos < interactSettings.actual.marginWidth) {
					if (yPos < interactSettings.actual.marginWidth) return HTTOPRIGHT;
					if (renderer.getSyncHeight() - yPos < interactSettings.actual.marginWidth) return HTBOTTOMRIGHT;
					return HTRIGHT;
				}
			}
			if (yPos < interactSettings.actual.marginWidth) return HTTOP;
			if (zoomed && renderer.getSyncHeight() - yPos < interactSettings.actual.marginWidth) return HTBOTTOM;
			if (yPos < interactSettings.actual.captionHeight) return HTCAPTION;
			LRESULT lr = 0;
			DwmDefWindowProc(hwnd, uMsg, wParam, lParam, &lr);
			return HTCLIENT;
		}
			[[likely]]
		case WM_MOUSEMOVE:
			interactManager.updateMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			UNREFERENCED_PARAMETER(game.passEvent(MouseActionCode::MAC_MOVE, 0, interactManager.getMouseX(), interactManager.getMouseY()));
			break;
			[[likely]]
		case WM_NCMOUSEMOVE: {
			POINT pt{ .x = GET_X_LPARAM(lParam), .y = GET_Y_LPARAM(lParam) };
			ScreenToClient(MainWindowHandle, &pt);
			interactManager.updateMouse(pt.x, pt.y);
			UNREFERENCED_PARAMETER(game.passEvent(MouseActionCode::MAC_MOVE, 0, interactManager.getMouseX(), interactManager.getMouseY()));
			break;
		}
		case WM_SIZE:
			renderer.syncSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			switch (wParam) {
				case SIZE_RESTORED:
				case SIZE_MAXIMIZED:
					renderer.resizeEnd();
					interactSettings.setScreenScale(static_cast<double>(GetSystemMetrics(SM_CYSCREEN)) / 2160.);
					break;
				case SIZE_MINIMIZED:
				case SIZE_MAXSHOW:
				case SIZE_MAXHIDE:
				default:
					break;
			}
			break;
		case WM_KEYDOWN:
			interactManager.update(static_cast<int>(wParam), true);
			break;
		case WM_KEYUP:
			interactManager.update(static_cast<int>(wParam), false);
			break;
		case WM_SYSKEYDOWN:
			interactManager.update(static_cast<int>(wParam), true);
			break;
		case WM_SYSKEYUP:
			interactManager.update(static_cast<int>(wParam), false);
			break;
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
			if (interactManager.isInClientCaption()) {
				interactManager.update(VK_LBUTTON, true);
				if (game.passEvent(MouseActionCode::MAC_DOWN, interactManager.getMouseButtonCode() | static_cast<unsigned int>(MouseButtonCodeEnum::MBC_L_CHANGE), interactManager.getMouseX(), interactManager.getMouseY())) return 0;
			}
			break;
		case WM_RBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			if (interactManager.isInClientCaption()) {
				interactManager.update(VK_RBUTTON, true);
				game.passEvent(MouseActionCode::MAC_DOWN, interactManager.getMouseButtonCode() | static_cast<unsigned int>(MouseButtonCodeEnum::MBC_R_CHANGE), interactManager.getMouseX(), interactManager.getMouseY());
			}
			break;
		case WM_APP_LBUTTONUP:
			if (interactManager.isInClientCaption()) {
				interactManager.update(VK_LBUTTON, false);
				game.passEvent(MouseActionCode::MAC_UP, interactManager.getMouseButtonCode() | static_cast<unsigned int>(MouseButtonCodeEnum::MBC_L_CHANGE), interactManager.getMouseX(), interactManager.getMouseY());
			}
			break;
		case WM_RBUTTONUP:
		case WM_NCRBUTTONUP:
			if (interactManager.isInClientCaption()) {
				interactManager.update(VK_RBUTTON, false);
				game.passEvent(MouseActionCode::MAC_UP, interactManager.getMouseButtonCode() | static_cast<unsigned int>(MouseButtonCodeEnum::MBC_R_CHANGE), interactManager.getMouseX(), interactManager.getMouseY());
			}
			break;
		case WM_APP_MBUTTONDOWN:
			if (interactManager.isInClientCaption()) {
				if (static_cast<QWORD>(wParam) & 0x10u) interactManager.update(VK_MBUTTON, true);
				game.passEvent(MouseActionCode::MAC_DOWN, interactManager.getMouseButtonCode() | static_cast<unsigned int>(MouseButtonCodeEnum::MBC_M_CHANGE), interactManager.getMouseX(), interactManager.getMouseY());
			}
			break;
		case WM_MBUTTONUP:
		case WM_NCMBUTTONUP:
			if (interactManager.isInClientCaption()) {
				interactManager.update(VK_MBUTTON, false);
				game.passEvent(MouseActionCode::MAC_UP, interactManager.getMouseButtonCode() | static_cast<unsigned int>(MouseButtonCodeEnum::MBC_M_CHANGE), interactManager.getMouseX(), interactManager.getMouseY());
			}
			break;
		case WM_MOUSEWHEEL:
			interactManager.updateWheel(GET_WHEEL_DELTA_WPARAM(wParam) / 120);
			return 0;
		case WM_MOUSEHOVER:
			interactManager.mouseHover();
			break;
		case WM_NCMOUSELEAVE:
			interactManager.mouseLeaveCaption();
			if (!interactManager.isInWindow())
				UNREFERENCED_PARAMETER(game.passEvent(MouseActionCode::MAC_LEAVE, 0, interactManager.getMouseX(), interactManager.getMouseY()));
			break;
		case WM_MOUSELEAVE:
			interactManager.mouseLeaveClient();
			if (!interactManager.isInWindow())
				UNREFERENCED_PARAMETER(game.passEvent(MouseActionCode::MAC_LEAVE, 0, interactManager.getMouseX(), interactManager.getMouseY()));
			break;
		case WM_DWMCOMPOSITIONCHANGED: {
			constexpr MARGINS margins{
				.cxLeftWidth = 0,
				.cxRightWidth = 0,
				.cyTopHeight = 0,
				.cyBottomHeight = 0
			};
			UNREFERENCED_PARAMETER(RemoveDefaultCaption(hwnd, &margins));
			break;
		}
		case WM_NCCALCSIZE:
			if (wParam == 1) {
				NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
				params->rgrc[0].left = params->rgrc[0].left + 0;
				params->rgrc[0].top = params->rgrc[0].top + 0;
				params->rgrc[0].right = params->rgrc[0].right - 0;
				params->rgrc[0].bottom = params->rgrc[0].bottom - 0;
				renderer.resizeStart();
				return 0;
			}
		case WM_EXITSIZEMOVE:
		case WM_APP_EXITSIZEMOVE:
			renderer.resizeEnd();
			interactSettings.setScreenScale(static_cast<double>(GetSystemMetrics(SM_CYSCREEN)) / 2160.);
			break;
		case WM_APP_REQUESTHDC: {
			GdiRenderer& gdiRenderer = dynamic_cast<GdiRenderer&>(renderer);
			if (gdiRenderer.MainDC) DeleteDC(gdiRenderer.MainDC);
			gdiRenderer.MainDC = GetDC(MainWindowHandle);
			Logger.info(L"Get new DC: " + std::to_wstring(reinterpret_cast<QWORD>(gdiRenderer.MainDC)));
			if (!gdiRenderer.MainDC) Logger.error(L"WM_APP_REQUESTHDC failed to initialize HDC. LastError: " + std::to_wstring(GetLastError()));
			else SetBkMode(gdiRenderer.MainDC, TRANSPARENT);
			gdiRenderer.refreshedHDC = 1;
			break;
		}
			[[unlikely]]
		case WM_DESTROY:
			PostQuitMessage(0);
			Logger.warn(L"Window destroyed");
			isRunning = false;
			return 0;
			[[unlikely]]
		case WM_APP_GAMESTART:
			renderer.requireResize();
			return 0;
		default:
			break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT __stdcall HookProc(const int code, const WPARAM wParam, const LPARAM lParam) {
	if (code < 0) {
		Logger.log(L"HookProc nCode < 0");
		return CallNextHookEx(nullptr, code, wParam, lParam);
	}
	switch (const MSG* p = reinterpret_cast<MSG*>(lParam); p->message) {
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP:
			PostMessageW(MainWindowHandle, WM_APP_LBUTTONUP, p->wParam, p->lParam);
			Logger.trace(wParam ? L"[Hook] LButtonUp    (Removed)" : L"[Hook] LButtonUp");
			return 0;
		case WM_MBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			PostMessageW(MainWindowHandle, WM_APP_MBUTTONDOWN, p->wParam, p->lParam);
			Logger.trace(wParam ? L"[Hook] MButtonDown  (Removed)" : L"[Hook] MButtonDown");
			return 0;
		default:
			break;
	}
	return CallNextHookEx(nullptr, code, wParam, lParam);
}

int __stdcall MessageLoop() {
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

int __stdcall wWinMain(const HINSTANCE hInstance, const HINSTANCE, [[maybe_unused]] const LPWSTR lpCmdLine, [[maybe_unused]] const int nShowCmd) {
	Logger.info(L"wWinMain started");
	SetConsoleOutputCP(65001);
	SetUnhandledExceptionFilter(UnhandledExceptionFilter);
	translator.initialize();
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = L"None";
	wc.lpszClassName = ApplicationName.c_str();
	if (!RegisterClassExW(&wc)) return FALSE;
	if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) Logger.print(L"SetProcessDpiAwarenessContext failed. LastError:", GetLastError());
	MainInstance = hInstance;
	MainWindowHandle = CreateWindowExW(0, wc.lpszClassName, wc.lpszClassName, WS_VISIBLE | WS_MAXIMIZEBOX | WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);
	constexpr MARGINS margins{
		.cxLeftWidth = 0,
		.cxRightWidth = 0,
		.cyTopHeight = 0,
		.cyBottomHeight = 0
	};
	UNREFERENCED_PARAMETER(RemoveDefaultCaption(MainWindowHandle, &margins));
	// SetWindowLongW(MainWindowHandle, GWL_EXSTYLE, GetWindowLongW(MainWindowHandle, GWL_EXSTYLE) | WS_EX_LAYERED);
	// SetLayeredWindowAttributes(MainWindowHandle, 0xffffff, 0xe0, LWA_COLORKEY /* | LWA_ALPHA */);
	SetWindowPos(MainWindowHandle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	ShowWindow(MainWindowHandle, nShowCmd);
	const HHOOK hook = SetWindowsHookW(WH_GETMESSAGE, HookProc);
	if (!hook) Logger.error(Logger.of(L"SetWindowsHookW failed. LastError:", GetLastError()));
	{
		// int a = -40;
		// game.tasks.pushNewed(allocatedFor(new Task([&a](Task& self) {
		// 	if (a) SetLayeredWindowAttributes(MainWindowHandle, 0xffffff, static_cast<BYTE>(0xff * (40 + ++a) / 40), LWA_COLORKEY | LWA_ALPHA);
		// 	else {
		// 		SetLayeredWindowAttributes(MainWindowHandle, 0xffffff, 0xff, LWA_COLORKEY | LWA_ALPHA);
		// 		self.schedulePop(true);
		// 		SetWindowLongW(MainWindowHandle, GWL_EXSTYLE, GetWindowLongW(MainWindowHandle, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		// 	}
		// 	return 0;
		// 	})
		// ));
		game.initialize();
		interactManager.initialize();
		World* w = StartWorld::create();
		game.worldManager->addWorld(w);
		game.worldManager->setWorld(w);
		Player* p = Player::create(Vector2D(0.5, 0.5));
		game.entityManager->addEntity(p);
		w->addEntity(p, WorldTransportReason::InitialGeneration);
		renderer.getCamera().setTargetEntity(p);
		GameThread = Thread(gameThread);
		RenderThread = Thread(renderThread);
	}
	test();
	PostMessageW(MainWindowHandle, WM_APP_GAMESTART, 0, 0);
	const int ret = MessageLoop();
	renderThread();
	isRunning = false;
	if (GameThread.joinable()) GameThread.join();
	if (RenderThread.joinable()) RenderThread.join();
	Logger.info(L"Thread terminated");
	if (!UnhookWindowsHookEx(hook)) Logger.error(L"Failed to UnhookWindowsHookEx. LastError:" + std::to_wstring(GetLastError()));
	if (!UnregisterClassW(wc.lpszClassName, wc.hInstance)) Logger.error(L"Failed to UnregisterClassW. LastError:" + std::to_wstring(GetLastError()));
	renderer.finalize(false);
	return ret;
}
