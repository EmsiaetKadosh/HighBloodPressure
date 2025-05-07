//
// Created by EmsiaetKadosh on 25-1-18.
//

#pragma once

#include "Window.h"

class StartWindow final : public Window {
	TranslatableText title = TranslatableText(L"hbp.title");

	StartWindow() {
		Button* start = dynamic_cast<Button*>(widgets.emplace_back(std::move(Button(0, 0.1, 0.4, 0.08, UILocation::CENTER, L"hbp.button.exit"_translates))).ptr());
		start->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(L"hbp.float.exit"_translates.getRenderableString()); };
		start->mouseClick = [](Widget&, MouseButtonCode) {
			game.setWindow(&ConfirmWindow::of(L"hbp.confirming.exit"_translates)->requireCancel().requireConfirm([](Button& confirm) {
				confirm.mouseClick = [](Widget&, MouseButtonCode) { DestroyWindow(MainWindowHandle); };
				confirm.onTick = [](Widget& self, MouseButtonCode) { if (self.containsMouse()) self.backgroundColor.hover = static_cast<Button&>(self).animation.adaptsColor(0x99008800, 0x9900ff00); };
			}));
		};
		Button* optn = dynamic_cast<Button*>(widgets.emplace_back(std::move(Button(0, 0.2, 0.4, 0.08, UILocation::CENTER, L"hbp.button.settings"_translates))).ptr());
		optn->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(L"hbp.float.settings"_translates.getRenderableString()); };
		Button* exit = dynamic_cast<Button*>(widgets.emplace_back(std::move(Button(0, 0.3, 0.4, 0.08, UILocation::CENTER, L"hbp.button.start"_translates))).ptr());
		exit->onTick = [](const Widget& self, MouseButtonCode) { if (self.containsMouse()) game.getFloatWindow().push(L"hbp.float.start"_translates.getRenderableString()); };
		exit->mouseClick = [](Widget&, MouseButtonCode) {
			game.setWindow(&ConfirmWindow::of(L"hbp.confirming.start"_translates)->requireCancel().requireConfirm([](Button& confirm) {
				confirm.mouseClick = [](Widget&, MouseButtonCode) { DestroyWindow(MainWindowHandle); };
				confirm.onTick = [](Widget& self, MouseButtonCode) { if (self.containsMouse()) self.backgroundColor.hover = static_cast<Button&>(self).animation.adaptsColor(0x99008800, 0x9900ff00); };
			}));
		};
	}

public:
	static StartWindow* create() noexcept { return allocatedFor(new StartWindow()); }

	void render(const double tickDelta, QWORD tickRendering) const noexcept override {
		renderer.getFontManager().getDefault().drawCenter(title.getRenderableString(), 0, 0, renderer.getWidth(), renderer.getHeight());
		Window::render(tickDelta, tickRendering);
	}
};

class SettingsWindow final : public Window {
public:
	SettingsWindow() {
	}
};
