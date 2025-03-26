//
// Created by EmsiaetKadosh on 25-1-14.
//

#include "Window.h"

#include "..\game\Animation.h"
#include "..\game\Game.h"
#include "..\interact\InteractManager.h"

int Window::pop() noexcept {
	gc.submit(this);
	Success();
}

void Window::render(const double tickDelta) const noexcept { for (const Widget* widget : widgets) widget->render(tickDelta); }

void Window::tick() noexcept {
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
	Widget* close = widgets.emplace_back(Button(0, 0, interactSettings.actual.captionHeight, interactSettings.actual.captionHeight, UILocation::RIGHT_TOP, L"\\f\1\u2716"_literal));
	close->mouseClick = [](Widget&, MouseButtonCode) { DestroyWindow(MainWindowHandle); };
	close->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(TranslatableText(L"hbp.caption.close").getRenderableString()); };
	close->absolute();
	close->backgroundColor.hover = 0xffee0000;
	close->backgroundColor.active = 0;
	close->backgroundColor.inactive = 0xffaaaaaa;
	close->backgroundColor.clicked = 0xffee8888;
	close->foregroundColor.hover = 0xff000000;
	close->foregroundColor.active = 0xff000000;
	close->foregroundColor.inactive = 0xff000000;
	close->foregroundColor.clicked = 0xff000000;

	Widget* maxRestore = widgets.emplace_back(Button(-interactSettings.actual.captionHeight, 0, interactSettings.actual.captionHeight, interactSettings.actual.captionHeight, UILocation::RIGHT_TOP, IsZoomed(MainWindowHandle) ? L"\\f\1🗗"_literal : L"\\f\1🗖"_literal));
	maxRestore->mouseClick = [](Widget&, MouseButtonCode) {};
	maxRestore->mouseClick = [](Widget& self, MouseButtonCode) {
		if ((self.unused[1] = static_cast<char>(IsZoomed(MainWindowHandle)))) ShowWindow(MainWindowHandle, SW_RESTORE);
		else ShowWindow(MainWindowHandle, SW_MAXIMIZE);
	};
	maxRestore->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(self.unused[1] ? TranslatableText(L"hbp.caption.maximize").getRenderableString() : TranslatableText(L"hbp.caption.restore").getRenderableString()); };
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

	Widget* hide = widgets.emplace_back(Button(-2 * interactSettings.actual.captionHeight, 0, interactSettings.actual.captionHeight, interactSettings.actual.captionHeight, UILocation::RIGHT_TOP, L"\\f\1🗕"_literal));
	hide->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(TranslatableText(L"hbp.caption.minimize").getRenderableString()); };
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

	Widget* options = widgets.emplace_back(Button(0, 0, interactSettings.actual.captionHeight, interactSettings.actual.captionHeight, UILocation::LEFT_TOP, L"\\f\1⛭"_literal));
	options->onTick = [](const Widget& self, MouseButtonCode) {
		if (self.containsMouse()) {
			game.getFloatWindow().push(TranslatableText(L"hbp.float.settings").getRenderableString());
			game.getFloatWindow().push(LiteralText(L"\\.ffee0000\\#ff000000中键调用GetLastError()").getRenderableString());
			game.getFloatWindow().push(TranslatableText(L"hbp.float.freshCanvas").getRenderableString());
		}
	};
	options->mouseClick = [](Widget&, const MouseButtonCode code) {
		if (static_cast<int>(MouseButtonCodeEnum::MBC_R_CHANGE) & code) {
			Logger.info(L"try resize: " + std::to_wstring(renderer.getWidth()) + L" " + std::to_wstring(renderer.getHeight()) + L", sync: " + std::to_wstring(renderer.getSyncWidth()) + L" " + std::to_wstring(renderer.getSyncHeight()));
			renderer.requireResize();
		} else if (static_cast<int>(MouseButtonCodeEnum::MBC_M_CHANGE) & code) Logger.info(L"LastError: " + std::to_wstring(GetLastError()));
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

void CaptionWindow::render(const double tickDelta) const noexcept {
	renderer.fill(0, 0, renderer.getWidth(), interactSettings.actual.captionHeight, 0xff666666);
	for (const Widget* widget : widgets) widget->render(tickDelta);
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

void FloatWindow::render(double tickDelta) const noexcept {
	if (not interactManager.isInWindow()) {
		strings.async();
		return;
	}
	if (strings.get().empty()) {
		strings.async();
		return;
	}
	x = interactManager.getMouseX();
	y = interactManager.getMouseY();
	int height = 0, width = 0;
	for (const RenderableString* str : strings.get()) {
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

	for (const RenderableString* str : strings.get()) {
		fontManager.getDefault().draw(*str, xf, yf);
		yf += str->getHeight();
	}

	strings.async();
}


unsigned int Widget::colorSelector(const Color& clr) const {
	if (!isActive) return clr.inactive;
	if (!hasMouse) return clr.active;
	if (hasMouseTrigger && (interactManager.getKey(VK_LBUTTON).isPressed() || interactManager.getKey(VK_RBUTTON).isPressed() || interactManager.getKey(VK_MBUTTON).isPressed())) return clr.clicked;
	return clr.hover;
}

void Widget::render(double tickDelta) const noexcept { renderer.fill(left, top, width, height, colorSelector(backgroundColor)); }

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
				top = static_cast<int>(y) + (renderer.getHeight() - height >> 1);
				break;
			case UILocation::LEFT_BOTTOM:
				left = static_cast<int>(x);
				top = static_cast<int>(y) + renderer.getHeight() - height;
				break;
			case UILocation::TOP:
				left = static_cast<int>(x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(y);
				break;
			case UILocation::CENTER:
				left = static_cast<int>(x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(y) + (renderer.getHeight() - height >> 1);
				break;
			case UILocation::BOTTOM:
				left = static_cast<int>(x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(y) + renderer.getHeight() - height;
				break;
			case UILocation::RIGHT_TOP:
				left = static_cast<int>(x) + renderer.getWidth() - width;
				top = static_cast<int>(y);
				break;
			case UILocation::RIGHT:
				left = static_cast<int>(x) + renderer.getWidth() - width;
				top = static_cast<int>(y) + (renderer.getHeight() - height >> 1);
				break;
			case UILocation::RIGHT_BOTTOM:
				left = static_cast<int>(x) + renderer.getWidth() - width;
				top = static_cast<int>(y) + renderer.getHeight() - height;
				break;
		}
	} else {
		width = static_cast<int>(renderer.getWidth() * w);
		height = static_cast<int>(renderer.getHeight() * h);
		switch (location) {
			case UILocation::LEFT_TOP:
				left = static_cast<int>(renderer.getWidth() * x);
				top = static_cast<int>(renderer.getHeight() * y);
				break;
			case UILocation::LEFT:
				left = static_cast<int>(renderer.getWidth() * x);
				top = static_cast<int>(renderer.getHeight() * y) + (renderer.getHeight() - height >> 1);
				break;
			case UILocation::LEFT_BOTTOM:
				left = static_cast<int>(renderer.getWidth() * x);
				top = static_cast<int>(renderer.getHeight() * y) + renderer.getHeight() - height;
				break;
			case UILocation::TOP:
				left = static_cast<int>(renderer.getWidth() * x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(renderer.getHeight() * y);
				break;
			case UILocation::CENTER:
				left = static_cast<int>(renderer.getWidth() * x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(renderer.getHeight() * y) + (renderer.getHeight() - height >> 1);
				break;
			case UILocation::BOTTOM:
				left = static_cast<int>(renderer.getWidth() * x) + (renderer.getWidth() - width >> 1);
				top = static_cast<int>(renderer.getHeight() * y) + renderer.getHeight() - height;
				break;
			case UILocation::RIGHT_TOP:
				left = static_cast<int>(renderer.getWidth() * x) + renderer.getWidth() - width;
				top = static_cast<int>(renderer.getHeight() * y);
				break;
			case UILocation::RIGHT:
				left = static_cast<int>(renderer.getWidth() * x) + renderer.getWidth() - width;
				top = static_cast<int>(renderer.getHeight() * y) + (renderer.getHeight() - height >> 1);
				break;
			case UILocation::RIGHT_BOTTOM:
				left = static_cast<int>(renderer.getWidth() * x) + renderer.getWidth() - width;
				top = static_cast<int>(renderer.getHeight() * y) + renderer.getHeight() - height;
				break;
		}
	}
}

void Button::render(const double tickDelta) const noexcept {
	Widget::render(tickDelta);
	if (name) fontManager.getDefault().drawCenter(name->getRenderableString(), left, top, width, height, colorSelector(foregroundColor));
}

ConfirmWindow& ConfirmWindow::requireConfirm(const Function<void(Button&)>& func) {
	confirm = dynamic_cast<Button*>(widgets.emplace_back(std::move(Button(0, 0, 0.4, 0.08, UILocation::CENTER, TranslatableText(L"hbp.confirm.confirm")))).ptr());
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
	cancel = dynamic_cast<Button*>(widgets.emplace_back(std::move(Button(0, 0.1, 0.4, 0.08, UILocation::CENTER, TranslatableText(L"hbp.confirm.cancel")))).ptr());
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

void ConfirmWindow::onClose() { pop(); }
