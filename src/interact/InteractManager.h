//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#include "..\global.hpp"

#include "..\def.h"

struct Keys {
	enum Enum : unsigned char {
		None = 0x00,
		LeftButton = 0x01,
		RightButton = 0x02,
		Cancel = 0x03,
		MiddleButton = 0x04,
		XButton1 = 0x05,
		XButton2 = 0x06,
		None_07 = 0x07,
		Backspace = 0x08,
		Tab = 0x09,
		None_0A = 0x0A,
		None_0B = 0x0B,
		Clear = 0x0C,
		Enter = 0x0D,
		None_0E = 0x0E,
		None_0F = 0x0F,
		Shift = 0x10,
		Ctrl = 0x11,
		Alt = 0x12,
		Pause = 0x13,
		CapsLock = 0x14,
		IME_KanaHangul = 0x15,
		IME_ON = 0x16,
		IME_Junja = 0x17,
		IME_Final = 0x18,
		IME_HanjaKanji = 0x19,
		IME_OFF = 0x1A,
		Escape = 0x1B,
		IME_Convert = 0x1C,
		IME_NonConvert = 0x1D,
		IME_Accept = 0x1E,
		IME_ModeChange = 0x1F,
		Space = 0x20,
		PageUp = 0x21,
		PageDown = 0x22,
		End = 0x23,
		Home = 0x24,
		Left = 0x25,
		Up = 0x26,
		Right = 0x27,
		Down = 0x28,
		Select = 0x29,
		Print = 0x2A,
		Execute = 0x2B,
		PrintScreen = 0x2C,
		Insert = 0x2D,
		Delete = 0x2E,
		Help = 0x2F,
		Key_0 = 0x30,
		Key_1 = 0x31,
		Key_2 = 0x32,
		Key_3 = 0x33,
		Key_4 = 0x34,
		Key_5 = 0x35,
		Key_6 = 0x36,
		Key_7 = 0x37,
		Key_8 = 0x38,
		Key_9 = 0x39,
		None_3A = 0x3A,
		None_3B = 0x3B,
		None_3C = 0x3C,
		None_3D = 0x3D,
		None_3E = 0x3E,
		None_3F = 0x3F,
		None_40 = 0x40,
		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4A,
		K = 0x4B,
		L = 0x4C,
		M = 0x4D,
		N = 0x4E,
		O = 0x4F,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5A,
		LeftWindows = 0x5B,
		RightWindows = 0x5C,
		Applications = 0x5D,
		None_5E = 0x5E,
		Sleep = 0x5F,
		NUMPAD_0 = 0x60,
		NUMPAD_1 = 0x61,
		NUMPAD_2 = 0x62,
		NUMPAD_3 = 0x63,
		NUMPAD_4 = 0x64,
		NUMPAD_5 = 0x65,
		NUMPAD_6 = 0x66,
		NUMPAD_7 = 0x67,
		NUMPAD_8 = 0x68,
		NUMPAD_9 = 0x69,
		Star = 0x6A,
		Plus = 0x6B,
		DotMid = 0x6C,
		Minus = 0x6D,
		Dot = 0x6E,
		Slash = 0x6F,
		F1 = 0x70,
		F2 = 0x71,
		F3 = 0x72,
		F4 = 0x73,
		F5 = 0x74,
		F6 = 0x75,
		F7 = 0x76,
		F8 = 0x77,
		F9 = 0x78,
		F10 = 0x79,
		F11 = 0x7A,
		F12 = 0x7B,
		F13 = 0x7C,
		F14 = 0x7D,
		F15 = 0x7E,
		F16 = 0x7F,
		F17 = 0x80,
		F18 = 0x81,
		F19 = 0x82,
		F20 = 0x83,
		F21 = 0x84,
		F22 = 0x85,
		F23 = 0x86,
		F24 = 0x87,
		None_88 = 0x88,
		None_89 = 0x89,
		None_8A = 0x8A,
		None_8B = 0x8B,
		None_8C = 0x8C,
		None_8D = 0x8D,
		None_8E = 0x8E,
		None_8F = 0x8F,
		NumLock = 0x90,
		ScrollLock = 0x91,
		None_92 = 0x92,
		None_93 = 0x93,
		None_94 = 0x94,
		None_95 = 0x95,
		None_96 = 0x96,
		None_97 = 0x97,
		None_98 = 0x98,
		None_99 = 0x99,
		None_9A = 0x9A,
		None_9B = 0x9B,
		None_9C = 0x9C,
		None_9D = 0x9D,
		None_9E = 0x9E,
		None_9F = 0x9F,
		Raw_LeftShift = 0xA0, // Raw-Input only. Use static LeftShift instead.
		Raw_RightShift = 0xA1, // Raw-Input only. Use static RightShift instead.
		Raw_LeftCtrl = 0xA2, // Raw-Input only. Use static LeftCtrl instead.
		Raw_RightCtrl = 0xA3, // Raw-Input only. Use static RightCtrl instead.
		Raw_LeftAlt = 0xA4, // Raw-Input only. Use static LeftAlt instead.
		Raw_RightAlt = 0xA5, // Raw-Input only. Use static RightAlt instead.
		BrowserBack = 0xA6,
		BrowserForward = 0xA7,
		BrowserRefresh = 0xA8,
		BrowserStop = 0xA9,
		BrowserSearch = 0xAA,
		BrowserFavorites = 0xAB,
		BrowserHome = 0xAC,
		VolumeMute = 0xAD,
		VolumeDown = 0xAE,
		VolumeUp = 0xAF,
		MediaNextTrack = 0xB0,
		MediaPrevTrack = 0xB1,
		MediaStop = 0xB2,
		MediaPlayPause = 0xB3,
		LaunchMail = 0xB4,
		LaunchMediaSelect = 0xB5,
		LaunchApp1 = 0xB6,
		LaunchApp2 = 0xB7,
		None_B8 = 0xB8,
		None_B9 = 0xB9,
		OEM_1_Semicolon_Colon = 0xBA,
		OEM_Plus = 0xBB,
		OEM_Comma = 0xBC,
		OEM_Minus = 0xBD,
		OEM_Dot = 0xBE,
		OEM_2_Slash_Question = 0xBF,
		OEM_3_Backquote_Tilde = 0xC0,
		None_C1 = 0xC1,
		None_C2 = 0xC2,
		None_C3 = 0xC3,
		None_C4 = 0xC4,
		None_C5 = 0xC5,
		None_C6 = 0xC6,
		None_C7 = 0xC7,
		None_C8 = 0xC8,
		None_C9 = 0xC9,
		None_CA = 0xCA,
		None_CB = 0xCB,
		None_CC = 0xCC,
		None_CD = 0xCD,
		None_CE = 0xCE,
		None_CF = 0xCF,
		None_D0 = 0xD0,
		None_D1 = 0xD1,
		None_D2 = 0xD2,
		None_D3 = 0xD3,
		None_D4 = 0xD4,
		None_D5 = 0xD5,
		None_D6 = 0xD6,
		None_D7 = 0xD7,
		None_D8 = 0xD8,
		None_D9 = 0xD9,
		None_DA = 0xDA,
		OEM_4_LBrackets = 0xDB,
		OEM_5_BackSlash_Vertical = 0xDC,
		OEM_6_RBrackets = 0xDD,
		OEM_7_Quotes = 0xDE,
		OEM_8 = 0xDF,
		None_E0 = 0xE0,
		None_E1 = 0xE1,
		OEM_102_More_Less = 0xE2,
		None_E3 = 0xE3,
		None_E4 = 0xE4,
		IME_ProcessKey = 0xE5,
		None_E6 = 0xE6,
		IME_Packet = 0xE7,
		None_E8 = 0xE8,
		None_E9 = 0xE9,
		None_EA = 0xEA,
		None_EB = 0xEB,
		None_EC = 0xEC,
		None_ED = 0xED,
		None_EE = 0xEE,
		None_EF = 0xEF,
		None_F0 = 0xF0,
		None_F1 = 0xF1,
		None_F2 = 0xF2,
		None_F3 = 0xF3,
		None_F4 = 0xF4,
		None_F5 = 0xF5,
		Attn = 0xF6,
		CrSel = 0xF7,
		ExSel = 0xF8,
		ErEof = 0xF9,
		Play = 0xFA,
		Zoom = 0xFB,
		None_FC = 0xFC,
		PA1 = 0xFD,
		OEM_Clear = 0xFE,
	};

private:
	friend class InteractManager;
	inline static Enum InnerLeftShift = Shift;
	inline static Enum InnerRightShift = Shift;
	inline static Enum InnerLeftCtrl = Ctrl;
	inline static Enum InnerRightCtrl = Ctrl;
	inline static Enum InnerLeftAlt = Alt;
	inline static Enum InnerRightAlt = Alt;
	Enum value;

public:
	inline static const Enum& LeftShift = InnerLeftShift;
	inline static const Enum& RightShift = InnerRightShift;
	inline static const Enum& LeftCtrl = InnerLeftCtrl;
	inline static const Enum& RightCtrl = InnerRightCtrl;
	inline static const Enum& LeftAlt = InnerLeftAlt;
	inline static const Enum& RightAlt = InnerRightAlt;

	Keys() : value(None) {}
	Keys(const Enum value) : value(value) {}
	Keys(const Keys&) noexcept = default;
	Keys(Keys&&) noexcept = default;
	Keys& operator=(const Keys&) noexcept = default;
	Keys& operator=(Keys&&) noexcept = default;
	~Keys() noexcept = default;

	operator unsigned char() const noexcept { return value; }
};

class KeyStatus {
	friend class InteractManager;
	String name;
	unsigned int pressTimes = 0;
	bool pressed = false;
	bool notDealt = false;

public:
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

struct KeyBindingID {
	unsigned int region;
	unsigned int key;
};

class KeyBindingClass {
	friend struct LessKeyBinding;
	friend class KeyRegionClass;
	friend class KeyBindingManager;
	const String id;
	const KeyRegionClass* region;
	unsigned int pressTimes = 0;
	Keys keyCode[8]{};

public:
	KeyBindingClass(String&& id, [[carlbeks::nonnull]] const KeyRegionClass* region, const Keys k1, const Keys k2, const Keys k3, const Keys k4, const Keys k5, const Keys k6, const Keys k7, const Keys k8) : id(std::move(id)), region(region), keyCode{k1, k2, k3, k4, k5, k6, k7, k8} {}

	void deals() const noexcept;
	[[nodiscard]] bool isPressed() const noexcept;
	[[nodiscard]] bool isPressedThenDeal() const noexcept;
	[[nodiscard]] unsigned int boundKeyCount() const noexcept;
	[[nodiscard, deprecated("not implemented")]] unsigned int wasPressed() const noexcept;
	[[nodiscard]] unsigned int wasPressedThenDeal() const noexcept;
	[[nodiscard]] String getId() const noexcept;
};

struct LessKeyBinding { // std::less
	using is_transparent = String;
	[[nodiscard]] bool operator()(const KeyBindingClass& lhs, const KeyBindingClass& rhs) const noexcept { return lhs.id < rhs.id; }
	[[nodiscard]] bool operator()(const String& lhs, const KeyBindingClass& rhs) const noexcept { return lhs < rhs.id; }
	[[nodiscard]] bool operator()(const KeyBindingClass& lhs, const String& rhs) const noexcept { return lhs.id < rhs; }
};

class KeyRegionClass {
	friend struct LessKeyRegion;
	friend class KeyBindingClass;
	friend class KeyBindingManager;
	mutable Set<KeyBindingClass, LessKeyBinding> keyBindings;
	String id;

public:
	KeyRegionClass(const String& id) : id(id) {}
	KeyRegionClass(String&& id) : id(std::move(id)) {}

	const KeyBindingClass& newBinding(String&& id, const Keys k1, const Keys k2 = Keys::None, const Keys k3 = Keys::None, const Keys k4 = Keys::None, const Keys k5 = Keys::None, const Keys k6 = Keys::None, const Keys k7 = Keys::None, const Keys k8 = Keys::None) const { return keyBindings.emplace(std::move(id), this, k1, k2, k3, k4, k5, k6, k7, k8).first.operator*(); }

	[[nodiscard]] const KeyBindingClass* getBinding(const String& id) const {
		if (const auto iter = keyBindings.find(id); iter != keyBindings.end()) return &*iter;
		return nullptr;
	}
};

struct LessKeyRegion {
	using is_transparent = String;
	[[nodiscard]] bool operator()(const KeyRegionClass& lhs, const KeyRegionClass& rhs) const noexcept { return lhs.id < rhs.id; }
	[[nodiscard]] bool operator()(const String& lhs, const KeyRegionClass& rhs) const noexcept { return lhs < rhs.id; }
	[[nodiscard]] bool operator()(const KeyRegionClass& lhs, const String& rhs) const noexcept { return lhs.id < rhs; }
};

using KeyBinding = const KeyBindingClass;
using KeyRegion = const KeyRegionClass;

class KeyBindingManager {
	friend class InteractManager;
	Set<KeyRegionClass, LessKeyRegion> keyRegions;
	Keys rebinding[8]{};

	KeyBindingManager() { keyRegions.emplace(KeyRegionClass(L"main")); }

public:
	KeyRegion& registerRegion(String&& description) noexcept { return keyRegions.emplace(std::move(description)).first.operator*(); }

	[[nodiscard]] KeyRegion& getRegion(const String& id) noexcept {
		if (const auto iter = keyRegions.find(id); iter != keyRegions.cend()) return *iter;
		return keyRegions.begin().operator*();
	}

	[[nodiscard]] KeyRegion& getOrNew(const String& id) noexcept {
		if (const auto iter = keyRegions.find(id); iter != keyRegions.cend()) return *iter;
		return keyRegions.emplace(id).first.operator*();
	}

	[[nodiscard]] KeyRegion& getOrNew(String&& id) noexcept {
		if (const auto iter = keyRegions.find(id); iter != keyRegions.cend()) return *iter;
		return keyRegions.emplace(std::move(id)).first.operator*();
	}
};

class InteractManager {
	KeyBindingManager keyBindings;
	Function<void()> inputListener;
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
	bool useRawInput = false;
	bool useIme = true;

public:
	void initialize() noexcept;
	InteractManager();
	void enableRawInput() noexcept;
	void update(unsigned char keyCode, bool isPressed) noexcept;
	void mouseLeaveCaption() noexcept;
	void mouseLeaveClient() noexcept;
	void updateMouse(int x, int y) noexcept;
	void updateWheel(const int wheel) noexcept { mouseWheel += wheel; }
	void mouseHover() noexcept { hovering = true; }
	void setInputMethodEditor(bool focusing = false) noexcept;
	[[nodiscard]] int getMouseX() const noexcept { return mouseX; }
	[[nodiscard]] int getMouseY() const noexcept { return mouseY; }
	[[nodiscard]] int getMouseWheel() const noexcept { return mouseWheel; }
	[[nodiscard]] bool rawInputEnabled() const noexcept { return useRawInput; }
	[[nodiscard]] bool isHovering() const noexcept { return hovering; }
	[[nodiscard]] bool isInWindow() const noexcept { return outsideWindow; }
	[[nodiscard]] bool isInSizeBox() const noexcept;
	[[nodiscard]] bool isInClient() const noexcept;
	[[nodiscard]] bool isInCaption() const noexcept;
	[[nodiscard]] bool isInClientCaption() const noexcept;
	[[nodiscard]] unsigned int /*MouseButtonCode*/ getMouseButtonCode() const noexcept;
	[[nodiscard]] KeyStatus& getKey(const Keys keyCode) noexcept { return keyStatus[keyCode]; }
	[[nodiscard]] KeyBindingManager& getKeyBindingManager() noexcept { return keyBindings; }
	int dealMouseWheel() noexcept;
};

class InteractSettings {
	[[carlbeks::predecl, carlbeks::defineat("renderer.h")]] friend class IRenderer;
	void resizeSetSystemScale(double scale);

public:
	/**
	 * @brief 该结构体存储用户设定以后，需要依赖其他值二次计算的项目
	 */
	struct Options {
		int captionHeight = 120;
		int marginWidth = 40;
		int fontHeight = 64;
		int floatWindowMargin = 16;
		unsigned int mapScale = 128; // 1格表现为mapScale像素
		double uiScale = 1; // UI的缩放比例
	};

	/**
	 * @brief 该结构体存储直接值
	 */
	struct Constants {
		double systemScale = 1; // 按照4K屏幕4096-2160的比例设定，在4K屏幕下适用systemScale = 1。其他情况的另算
		double smoothCamera = 0.9; // 相机平滑度。为0，始终瞬时设置相机的位置；为1，相机不动。
		static constexpr long long MsPerTick = 10;
		static constexpr long long MsPerRender = 1;
		long long msPerTick = 10;
		long long msPerRender = 1;
		unsigned int floatWindowBackground = 0xdd000000;
	};

	Options options; // 用户设置值
	Options actual; // 适应Scale后的实际值
	Constants constants;

	/**
	 * @note 该方法调用renderer.requireResize()。在resize时设置scale请调用private:resizeSetUiScale()
	 */
	void setUiScale(double scale);
	void setMapScale(unsigned int scale);
	void setScreenScale(double scale);
	void modifyUiScale(double scale);
	void modifyMapScale(int scale);
};
