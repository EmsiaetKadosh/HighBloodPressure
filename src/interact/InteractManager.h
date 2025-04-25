//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#include "..\hbp.h"
#include "..\def.h"

struct KeyStatus {
	String name;
	unsigned int pressTimes = 0;
	bool pressed = false;
	bool notDealt = false;

	[[nodiscard]] bool isPressed() const noexcept { return pressed; }
	[[nodiscard]] unsigned int wasPressed() const noexcept { return pressTimes; }
	[[nodiscard]] bool isPressedAndDeal() noexcept { return notDealt = false, pressed; }
	[[nodiscard]] bool wasPressedAndDeal() noexcept {
		const unsigned int ret = pressTimes;
		notDealt = false;
		pressTimes = 0;
		return ret;
	}

	void deals() noexcept {
		notDealt = false;
		pressTimes = 0;
	}

	[[nodiscard]] String toString() const noexcept { return L"KeyStatus: { name = \"" + name + L"\"; pressTimes = " + std::to_wstring(pressTimes) + L"; pressed = " + (pressed ? L"true; }" : L"false; }"); }
};

struct MouseStatus {
	String name;
	unsigned int pressTimes = 0;
	bool pressed = false;
	bool notDealt = false;
	bool longHold = false;

	[[nodiscard]] bool isPressed() const noexcept { return pressed; }
	[[nodiscard]] unsigned int wasPressed() const noexcept { return pressTimes; }

	void deals() noexcept {
		notDealt = false;
		pressTimes = 0;
	}

	[[nodiscard]] String toString() const noexcept { return L"MouseStatus: { name = \"" + name + L"\"; pressTimes = " + std::to_wstring(pressTimes) + L"; pressed = " + (pressed ? L"true, longHold = " : L"false, longHold = ") + (longHold ? L"true; }" : L"false; }"); }
};

struct KeyBindingLegacy;

class InteractManager {
	TRACKMOUSEEVENT trackMouseEvent{
		.cbSize = sizeof(TRACKMOUSEEVENT),
		.dwFlags = TME_HOVER | TME_LEAVE,
		.hwndTrack = nullptr,
		.dwHoverTime = HOVER_DEFAULT
	};
	KeyStatus keyStatus[256]{};
	int mouseX = 0, mouseY = 0;
	int mouseWheel = 0;
	int rebindResult = 0;
	char outsideWindow = 0; // 鼠标是否在窗口外部。1位：在客户区；2位：在标题栏
	bool rebinding = false;
	bool hovering = false;

public:
	void initialize() noexcept { trackMouseEvent.hwndTrack = MainWindowHandle; }
	InteractManager();

	void update(const int keyCode, const bool isPressed) noexcept {
		if (keyCode >= 256) return;
		if (rebinding) {
			rebindResult = keyCode;
			return;
		}
		keyStatus[keyCode].pressed = isPressed;
		if (isPressed) {
			keyStatus[keyCode].pressTimes++;
			keyStatus[keyCode].notDealt = true;
		}
	}


	void mouseLeaveCaption() noexcept {
		outsideWindow &= ~2;
		hovering = false;
	}

	void mouseLeaveClient() noexcept {
		outsideWindow &= ~1;
		hovering = false;
	}

	void updateMouse(int x, int y) noexcept;
	void updateWheel(const int wheel) noexcept { mouseWheel += wheel; }
	void mouseHover() noexcept { hovering = true; }
	[[nodiscard]] int getMouseX() const noexcept { return mouseX; }
	[[nodiscard]] int getMouseY() const noexcept { return mouseY; }
	[[nodiscard]] int getMouseWheel() const noexcept { return mouseWheel; }
	[[nodiscard]] bool isHovering() const noexcept { return hovering; }
	[[nodiscard]] bool isInWindow() const noexcept { return outsideWindow; }
	[[nodiscard]] bool isInSizeBox() const noexcept;
	[[nodiscard]] bool isInClientCaption() const noexcept;
	[[nodiscard]] KeyStatus& getKey(const int keyCode) noexcept { return keyStatus[keyCode]; }
	[[nodiscard]] KeyStatus& getKey(const KeyBindingLegacy& binding) noexcept;
	[[nodiscard]] unsigned int /*MouseButtonCode*/ getMouseButtonCode() const noexcept;

	int dealMouseWheel() noexcept {
		const int ret = mouseWheel;
		mouseWheel = 0;
		return ret;
	}
};

inline InteractManager interactManager = InteractManager();

struct KeyBindingLegacy {
	String id;
	int keyCode;
	[[nodiscard]] bool isPressed() const noexcept { return interactManager.getKey(keyCode).isPressed(); }
	[[nodiscard]] unsigned int wasPressed() const noexcept { return interactManager.getKey(keyCode).wasPressed(); }
	void deals() const noexcept { interactManager.getKey(keyCode).deals(); }
};

class KeyBinding;
struct LessKeyBinding;
class KeyRegion;
struct LessKeyRegion;
class KeyBindingManager;

class KeyBinding {
	friend struct LessKeyBinding;
	friend class KeyBindingManager;
	String id;
	unsigned int pressTimes = 0;
	unsigned char keyCode[8]{};

public:
	[[nodiscard]] bool isPressed() const noexcept {
		for (const int i : keyCode) if (!interactManager.getKey(i).isPressed()) return false;
		return true;
	}

	[[nodiscard]] unsigned int wasPressed() const noexcept { return pressTimes; }

	[[nodiscard]] unsigned int boundKeyCount() const noexcept {
		unsigned int ret = 0;
		while (keyCode[ret] && ret < 8) ++ret;
		return ret;
	}
};

struct LessKeyBinding { // std::less
	[[nodiscard]] bool operator()(const KeyBinding& lhs, const KeyBinding& rhs) const noexcept {
		const unsigned int lc = lhs.boundKeyCount(), rc = rhs.boundKeyCount();
		if (lc < rc) return false;
		if (lc > rc) return true;
		if (lhs.keyCode[0] < rhs.keyCode[0]) return true;
		if (lhs.keyCode[0] > rhs.keyCode[0]) return false;
		if (lhs.keyCode[1] == 0) return false;
		if (lhs.keyCode[1] < rhs.keyCode[1]) return true;
		if (lhs.keyCode[1] > rhs.keyCode[1]) return false;
		if (lhs.keyCode[2] == 0) return false;
		if (lhs.keyCode[2] < rhs.keyCode[2]) return true;
		if (lhs.keyCode[2] > rhs.keyCode[2]) return false;
		if (lhs.keyCode[3] == 0) return false;
		if (lhs.keyCode[3] < rhs.keyCode[3]) return true;
		if (lhs.keyCode[3] > rhs.keyCode[3]) return false;
		if (lhs.keyCode[4] == 0) return false;
		if (lhs.keyCode[4] < rhs.keyCode[4]) return true;
		if (lhs.keyCode[4] > rhs.keyCode[4]) return false;
		if (lhs.keyCode[5] == 0) return false;
		if (lhs.keyCode[5] < rhs.keyCode[5]) return true;
		if (lhs.keyCode[5] > rhs.keyCode[5]) return false;
		if (lhs.keyCode[6] == 0) return false;
		if (lhs.keyCode[6] < rhs.keyCode[6]) return true;
		if (lhs.keyCode[6] > rhs.keyCode[6]) return false;
		if (lhs.keyCode[7] == 0) return false;
		if (lhs.keyCode[7] < rhs.keyCode[7]) return true;
		return false;
	}
};

class KeyRegion {
	friend struct LessKeyRegion;
	friend class KeyBindingManager;
	const unsigned int idRegion;
	mutable Set<KeyBinding, LessKeyBinding> keyBindings;

	KeyRegion(const unsigned int id) : idRegion(id) {}
	void addKeyBinding(const KeyBinding& binding) const noexcept { keyBindings.insert(binding); }
	void addKeyBinding(KeyBinding&& binding) const noexcept { keyBindings.insert(std::move(binding)); }
};

struct LessKeyRegion {
	[[nodiscard]] bool operator()(const KeyRegion& lhs, const KeyRegion& rhs) const noexcept { return lhs.idRegion < rhs.idRegion; }
};

class KeyBindingManager {
	Set<KeyRegion, LessKeyRegion> keyRegions;
	unsigned int keyRegionCount = 0;

public:
	const KeyRegion& registerRegion() noexcept { return keyRegions.emplace(KeyRegion(++keyRegionCount)).first.operator*(); }
};

class InteractSettings {
	struct Options {
		int captionHeight = 120;
		int marginWidth = 40;
		int fontHeight = 64;
		int floatWindowMargin = 16;
		double mapScale = 128.0; // 1格表现为mapScale像素
	};

	struct Constants {
		/**
		 * 非常重要的设置项。<br>
		 * 管理ui的缩放比例。
		 */
		double uiScale = 1;
		double screenScale = 1;
		double smoothCamera = 0.9; // 相机平滑度。为0，始终瞬时设置相机的位置；为1，相机不动。
		long long msPerTick = 10;
		long long msPerRender = 1;
		unsigned int floatWindowBackground = 0xdd000000;
	};

public:
	Options options; // 用户设置值
	Options actual; // 适应Scale后的实际值
	Constants constants;

	InteractSettings& setUiScale(const double scale) noexcept {
		constants.uiScale = scale;
		actual.fontHeight = static_cast<int>(options.fontHeight * scale);
		return *this;
	}

	InteractSettings& setScreenScale(const double scale) noexcept {
		constants.screenScale = scale;
		actual.captionHeight = static_cast<int>(options.captionHeight * scale);
		actual.marginWidth = static_cast<int>(options.marginWidth * scale);
		return *this;
	}
};

inline InteractSettings interactSettings = InteractSettings();
