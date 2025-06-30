//
// Created by EmsiaetKadosh on 25-1-14.
//


#include "..\game\Animation.h"
#include "..\interact\InteractManager.h"
#include "Window.h"
#include "..\game\Game.h"
#include "..\game\world\World.h"
#include "xWindows.h"

int Widget::passEvent(const MouseActionCode action, const MouseButtonCode value, const int x, const int y) noexcept {
	if (action == MouseActionCode::MAC_LEAVE || !isMouseIn(x, y)) {
		if (hasMouse) onMouseLeave(0);
		hasMouse = false;
		return 0;
	}
	hasMouse = true;
	switch (action) {
		case MouseActionCode::MAC_HOVER:
			onHover(1);
			break;
		case MouseActionCode::MAC_MOVE:
			onHover(0);
			break;
		case MouseActionCode::MAC_DOWN:
			onMouseDown(value);
			break;
		case MouseActionCode::MAC_UP:
			onMouseUp(value);
			break;
		case MouseActionCode::MAC_DOUBLE:
			onMouseClick(0x80);
			break;
		default:
			break;
	}
	return 1;
}

int Window::pop() noexcept {
	gc.submit(this);
	Success();
}

void Window::render(const double tickDelta, const QWORD tickRendering) const noexcept { for (const Widget* widget : widgets) widget->render(tickDelta, tickRendering); }

void Window::tick() noexcept(false) {
	for (Widget* widget : widgets) widget->tick();
	if (reserved[0]) game.closeWindow(this);
}

void Window::onResize() { for (Widget* widget : widgets) widget->onResize(); }

int Window::passEvent(const MouseActionCode action, const MouseButtonCode value, const int x, const int y) noexcept {
	int ret = 0;
	for (Widget* widget : widgets) ret |= widget->passEvent(action, value, x, y);
	return ret;
}

int WindowManager::pop(Window* value) noexcept {
	if (value->list != static_cast<AnywhereEditableList*>(this)) {
		Logger.error(L"AnywhereEditableList::pop() : value is not in this list");
		Failed();
	}
	value->list = nullptr;
	value->next->prev = value->prev;
	value->prev->next = value->next;
	value->onClose();
	Success();
}

void WindowManager::clear() noexcept {
	for (Window& window : *this) {
		window.list = nullptr;
		window.onClose();
	}
	head.next = nullptr;
	tail.prev = nullptr;
}

CaptionWindow::CaptionWindow() {
	Button* close;
	widgets.emplace_back(&close, 0, 0, interactSettings.actual.captionHeight, interactSettings.actual.captionHeight, UILocation::RIGHT_TOP, L"\\f\1\u2716"_literal);
	close->mouseClick = [](Widget&, MouseButtonCode) { DestroyWindow(MainWindowHandle); };
	close->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(L"hbp.caption.close"_translates.getRenderableString()); };
	close->absolute();
	close->backgroundColor.hover = 0xffee0000;
	close->backgroundColor.active = 0;
	close->backgroundColor.inactive = 0xffaaaaaa;
	close->backgroundColor.clicked = 0xffee8888;
	close->foregroundColor.hover = 0xff000000;
	close->foregroundColor.active = 0xff000000;
	close->foregroundColor.inactive = 0xff000000;
	close->foregroundColor.clicked = 0xff000000;

	Button* maxRestore;
	widgets.emplace_back(&maxRestore, -interactSettings.actual.captionHeight, 0, interactSettings.actual.captionHeight, interactSettings.actual.captionHeight, UILocation::RIGHT_TOP, IsZoomed(MainWindowHandle) ? L"\\f\1🗗"_literal : L"\\f\1🗖"_literal);
	maxRestore->mouseClick = [](Widget&, MouseButtonCode) { renderer.setZoom(!renderer.isZoomed()); };
	maxRestore->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(renderer.isZoomed() ? L"hbp.caption.restore"_translates.getRenderableString() : L"hbp.caption.maximize"_translates.getRenderableString()); };
	maxRestore->absolute();
	maxRestore->unused[1] = static_cast<char>(IsZoomed(MainWindowHandle));
	maxRestore->backgroundColor.hover = 0xffcccccc;
	maxRestore->backgroundColor.active = 0;
	maxRestore->backgroundColor.inactive = 0xff555555;
	maxRestore->backgroundColor.clicked = 0xffaaaaaa;
	maxRestore->foregroundColor.hover = 0xff000000;
	maxRestore->foregroundColor.active = 0xff000000;
	maxRestore->foregroundColor.inactive = 0xff000000;
	maxRestore->foregroundColor.clicked = 0xff000000;

	Button* hide;
	widgets.emplace_back(&hide, -2 * interactSettings.actual.captionHeight, 0, interactSettings.actual.captionHeight, interactSettings.actual.captionHeight, UILocation::RIGHT_TOP, L"\\f\1🗕"_literal);
	hide->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(L"hbp.caption.minimize"_translates.getRenderableString()); };
	hide->mouseClick = [](Widget&, MouseButtonCode) { ShowWindow(MainWindowHandle, SW_MINIMIZE); };
	hide->absolute();
	hide->backgroundColor.hover = 0xffcccccc;
	hide->backgroundColor.active = 0;
	hide->backgroundColor.inactive = 0xff555555;
	hide->backgroundColor.clicked = 0xffaaaaaa;
	hide->foregroundColor.hover = 0xff000000;
	hide->foregroundColor.active = 0xff000000;
	hide->foregroundColor.inactive = 0xff000000;
	hide->foregroundColor.clicked = 0xff000000;

	Button* options;
	widgets.emplace_back(&options, 0, 0, interactSettings.actual.captionHeight, interactSettings.actual.captionHeight, UILocation::LEFT_TOP, L"\\f\1⛭"_literal);
	options->onTick = [](const Widget& self, MouseButtonCode) {
		if (self.containsMouse()) {
			game.getFloatWindow().push(L"hbp.float.settings"_translates.getRenderableString());
			game.getFloatWindow().push(L"hbp.float.freshCanvas"_translates.getRenderableString());
			game.getFloatWindow().push(L"\\f\2TM\\f\3TM\\f\4TM\\f\5TM");
		}
	};
	options->mouseClick = [](Widget&, const MouseButtonCode code) {
		if (static_cast<int>(MouseButtonCodeEnum::MBC_R_CHANGE) & code) {
			Logger.info(L"try resize: " + std::to_wstring(renderer.getWidth()) + L" " + std::to_wstring(renderer.getHeight()) + L", sync: " + std::to_wstring(renderer.getSyncWidth()) + L" " + std::to_wstring(renderer.getSyncHeight()));
			renderer.requireResize();
		} else if (static_cast<int>(MouseButtonCodeEnum::MBC_M_CHANGE) & code) {
			Logger.info(L"LastError: " + std::to_wstring(GetLastError()));
			Logger.info(game.getEntityManager().getEntity(1)->getLocation().getPosition().toString());
			if (interactSettings.constants.msPerTick < 100) interactSettings.constants.msPerTick = 40 * InteractSettings::Constants::MsPerTick;
			else interactSettings.constants.msPerTick = InteractSettings::Constants::MsPerTick;
		} else if (static_cast<int>(MouseButtonCodeEnum::MBC_L_CHANGE) & code) { if (!game.getWindow() && game.setWindow(SettingsWindow::create())) Logger.error(L"Failed to open SettingsWindow"); }
	};
	options->absolute();
	options->backgroundColor.hover = 0xffcccccc;
	options->backgroundColor.active = 0;
	options->backgroundColor.inactive = 0xff555555;
	options->backgroundColor.clicked = 0xffaaaaaa;
	options->foregroundColor.hover = 0xff000000;
	options->foregroundColor.active = 0xff000000;
	options->foregroundColor.inactive = 0xff000000;
	options->foregroundColor.clicked = 0xff000000;
}

bool CaptionWindow::onOpen() { throw InvalidOperationException(L"Should not open CaptionWindow"); }
void CaptionWindow::onClose() { throw InvalidOperationException(L"Should not close CaptionWindow"); }

void CaptionWindow::render(const double tickDelta, const QWORD tickRendering) const noexcept {
	if (hidden) return;
	renderer.fill(0, 0, renderer.getWidth(), interactSettings.actual.captionHeight, 0xff666666);
	for (const Widget* widget : widgets) widget->render(tickDelta, tickRendering);
	fontManager.get(1).drawCenter(L"High Blood Pressure", 0, 0, renderer.getWidth(), interactSettings.actual.captionHeight, 0xffee0000);
}

void CaptionWindow::onResize() {
	int left = 0, right = 0;
	for (Widget* widget : widgets) {
		widget->w = interactSettings.actual.captionHeight;
		widget->h = interactSettings.actual.captionHeight;
		switch (widget->location) {
			case UILocation::LEFT_TOP:
				widget->x = left;
				left += interactSettings.actual.captionHeight;
				break;
			case UILocation::RIGHT_TOP:
				widget->x = right;
				right -= interactSettings.actual.captionHeight;
				break;
			default:
				break;
		}
	}
	Window::onResize();
}

void FloatWindow::clear() noexcept {
	flag.atomicAcquire();
	lastTickStrings.swap(strings);
	strings.clear();
	thisTick = game.getTick();
	flag.atomicRelease();
}

void FloatWindow::render(double tickDelta, const QWORD tickRendering) const noexcept {
	if (not interactManager.isInWindow()) return;
	const List<Container<RenderableString>>& list = tickRendering > thisTick ? this->strings : this->lastTickStrings;
	if (list.empty()) return;
	x = interactManager.getMouseX();
	y = interactManager.getMouseY();
	int height = 0, width = 0;
	flag.atomicAcquire(true);
	for (const RenderableString* str : list) {
		height += str->getHeight();
		if (str->getWidth() > width) width = str->getWidth();
	}
	const int fwm2 = interactSettings.actual.floatWindowMargin * 2;
	x += fwm2; // 做一个偏移。不让小窗左下角直接对准鼠标
	width += fwm2;
	height += fwm2;
	if (x + width > renderer.getWidth()) x = renderer.getWidth() - width;
	y = y < height + interactSettings.actual.captionHeight ? interactSettings.actual.captionHeight : y - height;
	renderer.fill(x, y, width, height, interactSettings.constants.floatWindowBackground);

	const int xf = x + interactSettings.actual.floatWindowMargin;
	int yf = y + interactSettings.actual.floatWindowMargin;

	for (const RenderableString* str : list) {
		fontManager.getDefault().draw(*str, xf, yf);
		yf += str->getHeight();
	}
	flag.atomicRelease(true);
}


unsigned int Widget::colorSelector(const Color& clr) const {
	if (!isActive) return clr.inactive;
	if (!hasMouse) return clr.active;
	if (hasMouseTrigger && (interactManager.getKey(Keys::LeftButton).isPressed() || interactManager.getKey(Keys::RightButton).isPressed() || interactManager.getKey(Keys::MiddleButton).isPressed())) return clr.clicked;
	return clr.hover;
}

void Widget::render(double tickDelta, QWORD tickRendering) const noexcept { renderer.fill(left, top, width, height, colorSelector(backgroundColor)); }

Widget& Widget::absolute(const bool value) & {
	if (value == isAbsoluteLocation) return *this;
	isAbsoluteLocation = value;
	if (renderer.getWidth() != 0 && renderer.getHeight() != 0) onResize();
	return *this;
}

bool Widget::isMouseIn(int x, int y) noexcept {
	x -= left;
	y -= top;
	return 0 <= x and x <= width and 0 <= y and y <= height;
}

void Widget::onResize() {
	if (isAbsoluteLocation) {
		width = static_cast<int>(w);
		height = static_cast<int>(h);
		switch (location) {
			case UILocation::LEFT_TOP:
				left = static_cast<int>(x);
				top = static_cast<int>(y);
				break;
			case UILocation::LEFT:
				left = static_cast<int>(x);
				top = static_cast<int>(y) + (renderer.getClientHeight() - height >> 1);
				break;
			case UILocation::LEFT_BOTTOM:
				left = static_cast<int>(x);
				top = static_cast<int>(y) + renderer.getClientHeight() - height;
				break;
			case UILocation::TOP:
				left = static_cast<int>(x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(y);
				break;
			case UILocation::CENTER:
				left = static_cast<int>(x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(y) + (renderer.getClientHeight() - height >> 1);
				break;
			case UILocation::BOTTOM:
				left = static_cast<int>(x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(y) + renderer.getClientHeight() - height;
				break;
			case UILocation::RIGHT_TOP:
				left = static_cast<int>(x) + renderer.getWidth() - width;
				top = static_cast<int>(y);
				break;
			case UILocation::RIGHT:
				left = static_cast<int>(x) + renderer.getWidth() - width;
				top = static_cast<int>(y) + (renderer.getClientHeight() - height >> 1);
				break;
			case UILocation::RIGHT_BOTTOM:
				left = static_cast<int>(x) + renderer.getWidth() - width;
				top = static_cast<int>(y) + renderer.getClientHeight() - height;
				break;
		}
	} else {
		width = static_cast<int>(renderer.getWidth() * w);
		height = static_cast<int>(renderer.getClientHeight() * h);
		switch (location) {
			case UILocation::LEFT_TOP:
				left = static_cast<int>(renderer.getWidth() * x);
				top = static_cast<int>(renderer.getClientHeight() * y);
				break;
			case UILocation::LEFT:
				left = static_cast<int>(renderer.getWidth() * x);
				top = static_cast<int>(renderer.getClientHeight() * y) + (renderer.getClientHeight() - height >> 1);
				break;
			case UILocation::LEFT_BOTTOM:
				left = static_cast<int>(renderer.getWidth() * x);
				top = static_cast<int>(renderer.getClientHeight() * y) + renderer.getClientHeight() - height;
				break;
			case UILocation::TOP:
				left = static_cast<int>(renderer.getWidth() * x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(renderer.getClientHeight() * y);
				break;
			case UILocation::CENTER:
				left = static_cast<int>(renderer.getWidth() * x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(renderer.getClientHeight() * y) + (renderer.getClientHeight() - height >> 1);
				break;
			case UILocation::BOTTOM:
				left = static_cast<int>(renderer.getWidth() * x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(renderer.getClientHeight() * y) + renderer.getClientHeight() - height;
				break;
			case UILocation::RIGHT_TOP:
				left = static_cast<int>(renderer.getWidth() * x) + renderer.getWidth() - width;
				top = static_cast<int>(renderer.getClientHeight() * y);
				break;
			case UILocation::RIGHT:
				left = static_cast<int>(renderer.getWidth() * x) + renderer.getWidth() - width;
				top = static_cast<int>(renderer.getClientHeight() * y) + (renderer.getClientHeight() - height >> 1);
				break;
			case UILocation::RIGHT_BOTTOM:
				left = static_cast<int>(renderer.getWidth() * x) + renderer.getWidth() - width;
				top = static_cast<int>(renderer.getClientHeight() * y) + renderer.getClientHeight() - height;
				break;
		}
	}
}

void Button::render(const double tickDelta, const QWORD tickRendering) const noexcept {
	Widget::render(tickDelta, tickRendering);
	if (name) fontManager.getDefault().drawCenter(name->getRenderableString(), left, top, width, height, colorSelector(foregroundColor));
}

void TextBar::render(const double tickDelta, const QWORD tickRendering) const noexcept {
	Widget::render(tickDelta, tickRendering);
	editor.render(tickDelta, tickRendering);
}

void TextBar::onMouseDown(const MouseButtonCode code) noexcept {
	Widget::onMouseDown(code);
	editor.focus(true);
}

int TextBar::passEvent(const MouseActionCode action, const MouseButtonCode value, const int x, const int y) noexcept {
	if (action == MouseActionCode::MAC_DOWN && !isMouseIn(x, y)) editor.focus(false);
	return Widget::passEvent(action, value, x, y);
}

void ConfirmWindow::render(const double tickDelta, const QWORD tickRendering) const noexcept {
	int w, h;
	w = renderer.getWidth(), h = renderer.getHeight();
	w >>= 2, h >>= 2;
	renderer.fill(w, h, w + w, h + h, 0xcc222222);
	fontManager.getDefault().drawCenter(text->getRenderableString(), w, h, w + w, h + (h >> 1), 0xffeeeeee);
	for (const Container<Widget>& widget : widgets) widget->render(tickDelta, tickRendering);
}

ConfirmWindow& ConfirmWindow::requireConfirm(const Function<void(Button&)>& func) {
	widgets.emplace_back(&confirm, 0, 0, 0.4, 0.08, UILocation::CENTER, L"hbp.confirm.confirm"_translates);
	confirm->location = UILocation::CENTER;
	confirm->backgroundColor.active = 0x99000000;
	confirm->backgroundColor.hover = 0x9900ff00;
	confirm->backgroundColor.clicked = 0xff00ee00;
	confirm->foregroundColor.active = 0xff00ee00;
	confirm->foregroundColor.hover = 0xff000000;
	confirm->foregroundColor.clicked = 0xff000000;
	confirm->y = 0.21;
	confirm->h = 0.08;
	if (cancel) {
		confirm->w = 0.25;
		confirm->x = -0.125;
		cancel->x = 0.125;
		cancel->w = 0.25;
		cancel->onResize();
	} else {
		confirm->w = 0.5;
		confirm->x = 0;
	}
	confirm->onTick = [](Widget& confirm, MouseButtonCode) { if (confirm.containsMouse()) confirm.backgroundColor.hover = dynamic_cast<Button&>(confirm).animation.adaptsColor(0x99008800, 0x9900ff00); };
	confirm->mouseLeave = [](Widget& confirm, MouseButtonCode) { dynamic_cast<Button&>(confirm).animation.reset(); };
	if (func) func(*confirm);
	confirm->onResize();
	return *this;
}

ConfirmWindow& ConfirmWindow::requireCancel(const Function<void(Button&)>& func) {
	widgets.emplace_back(&cancel, 0, 0.1, 0.4, 0.08, UILocation::CENTER, L"hbp.confirm.cancel"_translates);
	cancel->mouseClick = [this](Widget&, MouseButtonCode) { syncClose(); };
	cancel->location = UILocation::CENTER;
	cancel->backgroundColor.active = 0x99000000;
	cancel->backgroundColor.hover = 0x99ff0000;
	cancel->backgroundColor.clicked = 0xffee0000;
	cancel->foregroundColor.active = 0xffee0000;
	cancel->foregroundColor.hover = 0xff000000;
	cancel->foregroundColor.clicked = 0xff000000;
	cancel->y = 0.21;
	cancel->h = 0.08;
	if (confirm) {
		cancel->x = 0;
		cancel->w = 0.25;
		confirm->x = -0.125;
		confirm->w = 0.25;
		confirm->onResize();
	} else {
		cancel->x = 0.125;
		cancel->w = 0.5;
	}
	cancel->onTick = [](Widget& cancel, MouseButtonCode) { if (cancel.containsMouse()) cancel.backgroundColor.hover = dynamic_cast<Button&>(cancel).animation.adaptsColor(0x99880000, 0x99ff0000); };
	cancel->mouseLeave = [](Widget& cancel, int) { dynamic_cast<Button&>(cancel).animation.reset(); };
	if (func) func(*cancel);
	cancel->onResize();
	return *this;
}
