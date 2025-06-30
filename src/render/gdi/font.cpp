//
// Created by EmsiaetKadosh on 25-6-18.
//

#pragma message("using GDI file " __FILE__)

#include "..\Renderer.h"
#include "..\..\utils\IText.h"
#include "font.hpp"
#include "renderer.hpp"

int GdiFont::drawSingle(const StringConfig& config, const int x, const int y, const unsigned int defaultColor) const noexcept {
	SelectObject(renderer->canvasDC, tryCreate(config));
	RECT rect{
		.left = x,
		.top = y + yOffsetPx,
		.right = 0,
		.bottom = 0
	};
	SetTextColor(renderer->canvasDC, config.isDefaultColor() ? defaultColor : renderer->changeColorFormat(config.color));
	DrawTextW(renderer->canvasDC, config.text.c_str(), static_cast<int>(config.text.length()), &rect, DT_SINGLELINE | DT_NOCLIP | DT_CALCRECT);
	if (!config.isDefaultBackground()) renderer->fill(&rect, config.background);
	DrawTextW(renderer->canvasDC, config.text.c_str(), static_cast<int>(config.text.length()), &rect, DT_SINGLELINE | DT_NOCLIP);
	return rect.right;
}

GdiFont::GdiFont(IRenderer* renderer, const FontID id, const String& name, const double heightModifier, const double yOffset, const long escapement, const long orientation, const bool adaptAllSize): IFonts(id, name, heightModifier, yOffset, escapement, orientation, adaptAllSize), renderer(assert_dynamic_cast<GdiRenderer*>(renderer)) {}
GdiFont::GdiFont(IRenderer* renderer, const FontID id, String&& name, const double heightModifier, const double yOffset, const long escapement, const long orientation, const bool adaptAllSize): IFonts(id, std::move(name), heightModifier, yOffset, escapement, orientation, adaptAllSize), renderer(assert_dynamic_cast<GdiRenderer*>(renderer)) {}

void GdiFont::drawDirect(const StringConfig& config, const int x, const int y, const unsigned int defaultColor) const noexcept {
	SelectObject(renderer->canvasDC, tryCreate(config));
	RECT rect{
		.left = x,
		.top = y + yOffsetPx,
		.right = 0,
		.bottom = 0
	};
	SetTextColor(renderer->canvasDC, config.isDefaultColor() ? defaultColor : renderer->changeColorFormat(config.color));
	if (!config.isDefaultBackground()) renderer->fill(&rect, config.background);
	DrawTextW(renderer->canvasDC, config.text.c_str(), static_cast<int>(config.text.length()), &rect, DT_SINGLELINE | DT_NOCLIP);
}

void GdiFont::clear() const noexcept {
	for (const auto& [_, fnt] : fonts) renderer->deleteObject(fnt);
	fonts.clear();
}

void GdiFont::draw(const RenderableString& text, int x, const int y, const unsigned int color) const noexcept {
	const COLORREF defaultColor = renderer->changeColorFormat(color);
	for (const StringConfig& config : text.configs) {
		if (config.idFont) {
			x = static_cast<GdiFont&>(fontManager.get(config.idFont)).drawSingle(config, x, y, defaultColor);
			continue;
		}
		SelectObject(renderer->canvasDC, tryCreate(config));
		RECT rect{
			.left = x,
			.top = y + yOffsetPx,
			.right = 0,
			.bottom = 0
		};
		SetTextColor(renderer->canvasDC, config.isDefaultColor() ? defaultColor : renderer->changeColorFormat(config.color));
		DrawTextW(renderer->canvasDC, config.text.c_str(), static_cast<int>(config.text.length()), &rect, DT_SINGLELINE | DT_NOCLIP | DT_CALCRECT);
		if (!config.isDefaultBackground()) renderer->fill(&rect, config.background);
		x = rect.right;
		DrawTextW(renderer->canvasDC, config.text.c_str(), static_cast<int>(config.text.length()), &rect, DT_SINGLELINE | DT_NOCLIP);
	}
}

void GdiFont::drawCenter(const RenderableString& text, int x, int y, const int w, const int h, const unsigned int color) const noexcept {
	const COLORREF defaultColor = renderer->changeColorFormat(color);
	using RenderConfig = RenderConfig;
	const QWORD size = text.configs.size();
	RenderConfig* configs = allocatedFor(new RenderConfig[size], sizeof(RenderConfig) * size);
	const int stringWidth = text.getWidth(configs, id);
	x += w - stringWidth >> 1;
	y += h - text.getHeight() >> 1;
	for (QWORD i = 0; i < size; ++i) {
		static_cast<const GdiFont*>(configs[i].font)->drawDirect(*configs[i].config, x, y, defaultColor);
		x += configs[i].width;
	}
	delete[] deallocating(configs);
}

int GdiFont::getWidth(const StringConfig& config) const noexcept {
	const HFONT font = tryCreate(config);
	RECT rect{};
	SelectObject(renderer->assistDC, font);
	DrawTextW(renderer->assistDC, config.text.c_str(), static_cast<int>(config.text.length()), &rect, DT_CALCRECT | DT_NOCLIP | DT_SINGLELINE);
	return rect.right;
}
