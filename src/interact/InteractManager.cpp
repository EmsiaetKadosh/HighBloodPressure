//
// Created by EmsiaetKadosh on 25-1-14.
//

#include "..\hbp.h"

#include "InteractManager.h"

#include "..\ui\Window.h"

void KeyBindingClass::deals() const noexcept {
	for (const auto& i : keyCode)
		if (i) interactManager.getKey(i).deals();
		else return;
}

bool KeyBindingClass::isPressed() const noexcept {
	for (const auto& i : keyCode) {
		if (!i) return true;
		if (!interactManager.getKey(i).isPressed()) return false;
	}
	return true;
}

bool KeyBindingClass::isPressedThenDeal() const noexcept {
	for (const auto& i : keyCode) {
		if (!i) break;
		if (!interactManager.getKey(i).isPressed()) return false;
	}
	deals();
	return true;
}

unsigned int KeyBindingClass::boundKeyCount() const noexcept {
	unsigned int ret = 0;
	while (keyCode[ret] && ret < 8) ++ret;
	return ret;
}

unsigned int KeyBindingClass::wasPressed() const noexcept {
	unsigned int min = 0, t = 0;
	for (const auto& i : keyCode) {
		if (!i) return min;
		if (t = interactManager.getKey(i).wasPressed(); !t) return 0;
		if (min == 0 || t < min) min = t;
	}
	return min;
}

unsigned int KeyBindingClass::wasPressedThenDeal() const noexcept {
	unsigned int min = 0, t = 0;
	for (const auto& i : keyCode) {
		if (!i) break;
		if (t = interactManager.getKey(i).wasPressed(); !t) return 0;
		if (min == 0 || t < min) min = t;
	}
	if (min) deals();
	return min;
}

String KeyBindingClass::getId() const noexcept { return L"key." + region->id + L"." + id; }

void InteractManager::initialize() noexcept {
	trackMouseEvent.hwndTrack = MainWindowHandle;
	KeyRegion& world = keyBindings.registerRegion(L"world");
	world.newBinding(L"move_left", Keys::A);
	world.newBinding(L"move_right", Keys::D);
	world.newBinding(L"move_jump", Keys::Space);
	world.newBinding(L"move_dodge", Keys::LeftShift);
	world.newBinding(L"pause", Keys::Escape);
	world.newBinding(L"speed_tweaker", Keys::Tab);
	KeyRegion& menu = keyBindings.registerRegion(L"menu");
	menu.newBinding(L"quit", Keys::Escape);
	interactManager.setInputMethodEditor(false);
}

void InteractManager::enableRawInput() noexcept {
	useRawInput = true;
	Keys::InnerLeftAlt = Keys::Raw_LeftAlt;
	Keys::InnerRightAlt = Keys::Raw_RightAlt;
	Keys::InnerLeftCtrl = Keys::Raw_LeftCtrl;
	Keys::InnerRightCtrl = Keys::Raw_RightCtrl;
	Keys::InnerLeftShift = Keys::Raw_LeftShift;
	Keys::InnerRightShift = Keys::Raw_RightShift;
}

void InteractManager::update(const unsigned char keyCode, const bool isPressed) noexcept {
	if (keyCode >= 256) return;
	if (rebinding) {
		rebindResult = keyCode;
		return;
	}
	auto& status = keyStatus[keyCode];
	if (!status.pressed) {
		++status.pressTimes;
		status.notDealt = true;
	}
	status.pressed = isPressed;
}

void InteractManager::mouseLeaveCaption() noexcept {
	outsideWindow &= ~2;
	hovering = false;
}

void InteractManager::mouseLeaveClient() noexcept {
	outsideWindow &= ~1;
	hovering = false;
}

InteractManager::InteractManager() {
	keyStatus[0x00].name = L"NONE";
	keyStatus[0x01].name = L"LeftButton";
	keyStatus[0x02].name = L"RightButton";
	keyStatus[0x03].name = L"Cancel";
	keyStatus[0x04].name = L"MiddleButton";
	keyStatus[0x05].name = L"XButton1";
	keyStatus[0x06].name = L"XButton2";
	keyStatus[0x07].name = L"None";
	keyStatus[0x08].name = L"Backspace";
	keyStatus[0x09].name = L"Tab";
	keyStatus[0x0A].name = L"None";
	keyStatus[0x0B].name = L"None";
	keyStatus[0x0C].name = L"Clear";
	keyStatus[0x0D].name = L"Enter";
	keyStatus[0x0E].name = L"None";
	keyStatus[0x0F].name = L"None";
	keyStatus[0x10].name = L"Shift";
	keyStatus[0x11].name = L"Ctrl";
	keyStatus[0x12].name = L"Alt";
	keyStatus[0x13].name = L"Pause";
	keyStatus[0x14].name = L"CapsLock";
	keyStatus[0x15].name = L"IME-KanaHangul";
	keyStatus[0x16].name = L"IME-ON";
	keyStatus[0x17].name = L"IME-Junja";
	keyStatus[0x18].name = L"IME-Final";
	keyStatus[0x19].name = L"IME-HanjaKanji";
	keyStatus[0x1A].name = L"IME-OFF";
	keyStatus[0x1B].name = L"Escape";
	keyStatus[0x1C].name = L"IME-Convert";
	keyStatus[0x1D].name = L"IME-NonConvert";
	keyStatus[0x1E].name = L"IME-Accept";
	keyStatus[0x1F].name = L"IME-ModeChange";
	keyStatus[0x20].name = L"Space";
	keyStatus[0x21].name = L"PageUp";
	keyStatus[0x22].name = L"PageDown";
	keyStatus[0x23].name = L"End";
	keyStatus[0x24].name = L"Home";
	keyStatus[0x25].name = L"Left";
	keyStatus[0x26].name = L"Up";
	keyStatus[0x27].name = L"Right";
	keyStatus[0x28].name = L"Down";
	keyStatus[0x29].name = L"Select";
	keyStatus[0x2A].name = L"Print";
	keyStatus[0x2B].name = L"Execute";
	keyStatus[0x2C].name = L"PrintScreen";
	keyStatus[0x2D].name = L"Insert";
	keyStatus[0x2E].name = L"Delete";
	keyStatus[0x2F].name = L"Help";
	keyStatus[0x30].name = L"0";
	keyStatus[0x31].name = L"1";
	keyStatus[0x32].name = L"2";
	keyStatus[0x33].name = L"3";
	keyStatus[0x34].name = L"4";
	keyStatus[0x35].name = L"5";
	keyStatus[0x36].name = L"6";
	keyStatus[0x37].name = L"7";
	keyStatus[0x38].name = L"8";
	keyStatus[0x39].name = L"9";
	keyStatus[0x3A].name = L"None";
	keyStatus[0x3B].name = L"None";
	keyStatus[0x3C].name = L"None";
	keyStatus[0x3D].name = L"None";
	keyStatus[0x3E].name = L"None";
	keyStatus[0x3F].name = L"None";
	keyStatus[0x40].name = L"None";
	keyStatus[0x41].name = L"A";
	keyStatus[0x42].name = L"B";
	keyStatus[0x43].name = L"C";
	keyStatus[0x44].name = L"D";
	keyStatus[0x45].name = L"E";
	keyStatus[0x46].name = L"F";
	keyStatus[0x47].name = L"G";
	keyStatus[0x48].name = L"H";
	keyStatus[0x49].name = L"I";
	keyStatus[0x4A].name = L"J";
	keyStatus[0x4B].name = L"K";
	keyStatus[0x4C].name = L"L";
	keyStatus[0x4D].name = L"M";
	keyStatus[0x4E].name = L"N";
	keyStatus[0x4F].name = L"O";
	keyStatus[0x50].name = L"P";
	keyStatus[0x51].name = L"Q";
	keyStatus[0x52].name = L"R";
	keyStatus[0x53].name = L"S";
	keyStatus[0x54].name = L"T";
	keyStatus[0x55].name = L"U";
	keyStatus[0x56].name = L"V";
	keyStatus[0x57].name = L"W";
	keyStatus[0x58].name = L"X";
	keyStatus[0x59].name = L"Y";
	keyStatus[0x5A].name = L"Z";
	keyStatus[0x5B].name = L"LeftWindows";
	keyStatus[0x5C].name = L"RightWindows";
	keyStatus[0x5D].name = L"Applications";
	keyStatus[0x5E].name = L"None";
	keyStatus[0x5F].name = L"Sleep";
	keyStatus[0x60].name = L"NUMPAD-0";
	keyStatus[0x61].name = L"NUMPAD-1";
	keyStatus[0x62].name = L"NUMPAD-2";
	keyStatus[0x63].name = L"NUMPAD-3";
	keyStatus[0x64].name = L"NUMPAD-4";
	keyStatus[0x65].name = L"NUMPAD-5";
	keyStatus[0x66].name = L"NUMPAD-6";
	keyStatus[0x67].name = L"NUMPAD-7";
	keyStatus[0x68].name = L"NUMPAD-8";
	keyStatus[0x69].name = L"NUMPAD-9";
	keyStatus[0x6A].name = L"*";
	keyStatus[0x6B].name = L"+";
	keyStatus[0x6C].name = L"·";
	keyStatus[0x6D].name = L"-";
	keyStatus[0x6E].name = L".";
	keyStatus[0x6F].name = L"/";
	keyStatus[0x70].name = L"F1";
	keyStatus[0x71].name = L"F2";
	keyStatus[0x72].name = L"F3";
	keyStatus[0x73].name = L"F4";
	keyStatus[0x74].name = L"F5";
	keyStatus[0x75].name = L"F6";
	keyStatus[0x76].name = L"F7";
	keyStatus[0x77].name = L"F8";
	keyStatus[0x78].name = L"F9";
	keyStatus[0x79].name = L"F10";
	keyStatus[0x7A].name = L"F11";
	keyStatus[0x7B].name = L"F12";
	keyStatus[0x7C].name = L"F13";
	keyStatus[0x7D].name = L"F14";
	keyStatus[0x7E].name = L"F15";
	keyStatus[0x7F].name = L"F16";
	keyStatus[0x80].name = L"F17";
	keyStatus[0x81].name = L"F18";
	keyStatus[0x82].name = L"F19";
	keyStatus[0x83].name = L"F20";
	keyStatus[0x84].name = L"F21";
	keyStatus[0x85].name = L"F22";
	keyStatus[0x86].name = L"F23";
	keyStatus[0x87].name = L"F24";
	keyStatus[0x88].name = L"None";
	keyStatus[0x89].name = L"None";
	keyStatus[0x8A].name = L"None";
	keyStatus[0x8B].name = L"None";
	keyStatus[0x8C].name = L"None";
	keyStatus[0x8D].name = L"None";
	keyStatus[0x8E].name = L"None";
	keyStatus[0x8F].name = L"None";
	keyStatus[0x90].name = L"NumLock";
	keyStatus[0x91].name = L"ScrollLock";
	keyStatus[0x92].name = L"None";
	keyStatus[0x93].name = L"None";
	keyStatus[0x94].name = L"None";
	keyStatus[0x95].name = L"None";
	keyStatus[0x96].name = L"None";
	keyStatus[0x97].name = L"None";
	keyStatus[0x98].name = L"None";
	keyStatus[0x99].name = L"None";
	keyStatus[0x9A].name = L"None";
	keyStatus[0x9B].name = L"None";
	keyStatus[0x9C].name = L"None";
	keyStatus[0x9D].name = L"None";
	keyStatus[0x9E].name = L"None";
	keyStatus[0x9F].name = L"None";
	keyStatus[0xA0].name = L"LeftShift";
	keyStatus[0xA1].name = L"RightShift";
	keyStatus[0xA2].name = L"LeftCtrl";
	keyStatus[0xA3].name = L"RightCtrl";
	keyStatus[0xA4].name = L"LeftAlt";
	keyStatus[0xA5].name = L"RightAlt";
	keyStatus[0xA6].name = L"BrowserBack";
	keyStatus[0xA7].name = L"BrowserForward";
	keyStatus[0xA8].name = L"BrowserRefresh";
	keyStatus[0xA9].name = L"BrowserStop";
	keyStatus[0xAA].name = L"BrowserSearch";
	keyStatus[0xAB].name = L"BrowserFavorites";
	keyStatus[0xAC].name = L"BrowserHome";
	keyStatus[0xAD].name = L"VolumeMute";
	keyStatus[0xAE].name = L"VolumeDown";
	keyStatus[0xAF].name = L"VolumeUp";
	keyStatus[0xB0].name = L"MediaNextTrack";
	keyStatus[0xB1].name = L"MediaPrevTrack";
	keyStatus[0xB2].name = L"MediaStop";
	keyStatus[0xB3].name = L"MediaPlayPause";
	keyStatus[0xB4].name = L"LaunchMail";
	keyStatus[0xB5].name = L"LaunchMediaSelect";
	keyStatus[0xB6].name = L"LaunchApp1";
	keyStatus[0xB7].name = L"LaunchApp2";
	keyStatus[0xB8].name = L"None";
	keyStatus[0xB9].name = L"None";
	keyStatus[0xBA].name = L"OEM-1;:";
	keyStatus[0xBB].name = L"OEM+";
	keyStatus[0xBC].name = L"OEM,";
	keyStatus[0xBD].name = L"OEM-";
	keyStatus[0xBE].name = L"OEM.";
	keyStatus[0xBF].name = L"OEM-2/?";
	keyStatus[0xC0].name = L"OEM-3`~";
	keyStatus[0xC1].name = L"None";
	keyStatus[0xC2].name = L"None";
	keyStatus[0xC3].name = L"None";
	keyStatus[0xC4].name = L"None";
	keyStatus[0xC5].name = L"None";
	keyStatus[0xC6].name = L"None";
	keyStatus[0xC7].name = L"None";
	keyStatus[0xC8].name = L"None";
	keyStatus[0xC9].name = L"None";
	keyStatus[0xCA].name = L"None";
	keyStatus[0xCB].name = L"None";
	keyStatus[0xCC].name = L"None";
	keyStatus[0xCD].name = L"None";
	keyStatus[0xCE].name = L"None";
	keyStatus[0xCF].name = L"None";
	keyStatus[0xD0].name = L"None";
	keyStatus[0xD1].name = L"None";
	keyStatus[0xD2].name = L"None";
	keyStatus[0xD3].name = L"None";
	keyStatus[0xD4].name = L"None";
	keyStatus[0xD5].name = L"None";
	keyStatus[0xD6].name = L"None";
	keyStatus[0xD7].name = L"None";
	keyStatus[0xD8].name = L"None";
	keyStatus[0xD9].name = L"None";
	keyStatus[0xDA].name = L"None";
	keyStatus[0xDB].name = L"OEM-4{[";
	keyStatus[0xDC].name = L"OEM-5|\\";
	keyStatus[0xDD].name = L"OEM-6}]";
	keyStatus[0xDE].name = L"OEM-7'\"";
	keyStatus[0xDF].name = L"OEM-8";
	keyStatus[0xE0].name = L"None";
	keyStatus[0xE1].name = L"None";
	keyStatus[0xE2].name = L"OEM-102<>";
	keyStatus[0xE3].name = L"None";
	keyStatus[0xE4].name = L"None";
	keyStatus[0xE5].name = L"IME-ProcessKey";
	keyStatus[0xE6].name = L"None";
	keyStatus[0xE7].name = L"IME-Packet";
	keyStatus[0xE8].name = L"None";
	keyStatus[0xE9].name = L"None";
	keyStatus[0xEA].name = L"None";
	keyStatus[0xEB].name = L"None";
	keyStatus[0xEC].name = L"None";
	keyStatus[0xED].name = L"None";
	keyStatus[0xEE].name = L"None";
	keyStatus[0xEF].name = L"None";
	keyStatus[0xF0].name = L"None";
	keyStatus[0xF1].name = L"None";
	keyStatus[0xF2].name = L"None";
	keyStatus[0xF3].name = L"None";
	keyStatus[0xF4].name = L"None";
	keyStatus[0xF5].name = L"None";
	keyStatus[0xF6].name = L"Attn";
	keyStatus[0xF7].name = L"CrSel";
	keyStatus[0xF8].name = L"ExSel";
	keyStatus[0xF9].name = L"ErEof";
	keyStatus[0xFA].name = L"Play";
	keyStatus[0xFB].name = L"Zoom";
	keyStatus[0xFC].name = L"None";
	keyStatus[0xFD].name = L"PA1";
	keyStatus[0xFE].name = L"OEM-Clear";
}

void InteractManager::updateMouse(const int x, const int y) noexcept {
	mouseX = x;
	mouseY = y;
	if (y < interactSettings.actual.captionHeight) outsideWindow = 2;
	else outsideWindow = 1;
	hovering = false;
	if (!TrackMouseEvent(&trackMouseEvent)) Logger.error(L"TrackMouseEvent failed. LastError: " + std::to_wstring(GetLastError()));
}

void InteractManager::setInputMethodEditor(const bool focusing) noexcept {
	static HIMC imc = nullptr;
	static DWORD conversionMode = 0, sentenceMode = 0;
	if (useIme == focusing) return;
	useIme = focusing;
	if (focusing) {
		Logger.debug(L"enable ime");
		if (imc) ImmSetConversionStatus(imc, conversionMode, sentenceMode);
		ImmAssociateContext(MainWindowHandle, imc);
		ImmSetOpenStatus(imc, true);
	} else if (!imc) {
		Logger.debug(L"disable ime");
		imc = ImmGetContext(MainWindowHandle);
		if (imc) ImmGetConversionStatus(imc, &conversionMode, &sentenceMode);
		ImmAssociateContext(MainWindowHandle, nullptr);
	}
}

bool InteractManager::isInSizeBox() const noexcept { return isInWindow() && !isInClientCaption(); }
bool InteractManager::isInClient() const noexcept { return mouseX > interactSettings.actual.marginWidth && mouseX < renderer.getSyncWidth() - interactSettings.actual.marginWidth && mouseY > interactSettings.actual.captionHeight && mouseY < renderer.getSyncHeight() - interactSettings.actual.marginWidth; }
bool InteractManager::isInCaption() const noexcept { return mouseX > interactSettings.actual.marginWidth && mouseX < renderer.getSyncWidth() - interactSettings.actual.marginWidth && mouseY > interactSettings.actual.marginWidth && mouseY < interactSettings.actual.captionHeight; }
bool InteractManager::isInClientCaption() const noexcept { return mouseX > interactSettings.actual.marginWidth && mouseX < renderer.getSyncWidth() - interactSettings.actual.marginWidth && mouseY > interactSettings.actual.marginWidth && mouseY < renderer.getSyncHeight() - interactSettings.actual.marginWidth; }

MouseButtonCode InteractManager::getMouseButtonCode() const noexcept { return (keyStatus[VK_LBUTTON].isPressed() ? static_cast<unsigned int>(MouseButtonCodeEnum::MBC_L_DOWN) : 0) | (keyStatus[VK_RBUTTON].isPressed() ? static_cast<int>(MouseButtonCodeEnum::MBC_R_DOWN) : 0) | (keyStatus[VK_MBUTTON].isPressed() ? static_cast<int>(MouseButtonCodeEnum::MBC_M_DOWN) : 0); }

int InteractManager::dealMouseWheel() noexcept {
	const int ret = mouseWheel;
	mouseWheel = 0;
	return ret;
}

void InteractSettings::resizeSetSystemScale(const double scale) {
	constants.systemScale = scale;
	actual.uiScale = scale * options.uiScale;
	actual.mapScale = scale * options.mapScale;
	actual.fontHeight = static_cast<int>(scale * options.fontHeight);
}

void InteractSettings::setUiScale(const double scale) {
	options.uiScale = scale;
	actual.uiScale = constants.systemScale * options.uiScale;
	renderer.requireResize();
}

void InteractSettings::setMapScale(const double scale) {
	options.mapScale = scale;
	actual.mapScale = constants.systemScale * options.mapScale;
}

void InteractSettings::setScreenScale(const double scale) {
	actual.captionHeight = static_cast<int>(options.captionHeight * scale);
	actual.marginWidth = static_cast<int>(options.marginWidth * scale);
}

