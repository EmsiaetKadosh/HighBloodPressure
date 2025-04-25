//
// Created by EmsiaetKadosh on 25-1-18.
//

#include "IText.h"

#include "..\def.h"
#include "..\render\Renderer.h"

int RenderableString::getHeight() const noexcept {
	int height = 0;
	for (const StringConfig& config : configs) if (const IFonts& font = renderer.getFontManager().get(config.idFont); font.getHeight() > height) height = font.getHeight();
	return height;
}

int RenderableString::getWidth(const FontID defaultID) const noexcept {
	int width = 0;
	for (const StringConfig& config : configs) width += renderer.getFontManager().get(config.idFont ? config.idFont : defaultID).getWidth(config);
	return width;
}

int RenderableString::getWidth(RenderConfig* renderConfigs, const FontID defaultID) const noexcept {
	int width = 0;
	int i = 0;
	for (const StringConfig& config : configs) {
		renderConfigs[i].config = &config;
		const IFonts& font = renderer.getFontManager().get(config.idFont ? config.idFont : defaultID);
		renderConfigs[i].font = &font;
		width += renderConfigs[i].width = font.getWidth(config);
		++i;
	}
	return width;
}

int GdiFont::drawSingle(const RenderableString::StringConfig& config, const int x, const int y, const unsigned int defaultColor) const {
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

void GdiFont::drawDirect(const RenderableString::StringConfig& config, const int x, const int y, const unsigned int defaultColor) const {
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

void GdiFont::clear() const {
	for (const auto& [_, fnt] : fonts) renderer->deleteObject(fnt);
	fonts.clear();
}

void GdiFont::draw(const RenderableString& text, int x, const int y, const unsigned int color) const {
	const COLORREF defaultColor = renderer->changeColorFormat(color);
	for (const RenderableString::StringConfig& config : text.configs) {
		if (config.idFont) {
			x = static_cast<GdiFont&>(renderer->fontManager->get(config.idFont)).drawSingle(config, x, y, defaultColor);
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

void GdiFont::drawCenter(const RenderableString& text, int x, int y, const int w, const int h, const unsigned int color) const {
	const COLORREF defaultColor = renderer->changeColorFormat(color);
	using RenderConfig = RenderableString::RenderConfig;
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

int GdiFont::getWidth(const RenderableString::StringConfig& config) const {
	const HFONT font = tryCreate(config);
	RECT rect{};
	SelectObject(renderer->assistDC, font);
	DrawTextW(renderer->assistDC, config.text.c_str(), static_cast<int>(config.text.length()), &rect, DT_CALCRECT | DT_NOCLIP | DT_SINGLELINE);
	return rect.right;
}

const String& TranslatableText::getText() const noexcept {
	refreshText();
	return target->getText();
}

const RenderableString& TranslatableText::getRenderableString() const noexcept {
	refreshText();
	return target->getRenderableString();
}

void TranslatableText::refreshText() const noexcept {
	if (target == nullptr || langConfig == translator.getConfigVersion()) {
		target = translator.getText(idSrc);
		langConfig = translator.getConfigVersion();
	}
}

void languageMakeChinese(Language& lang) {
	Logger.debug(L"languageMakeChinese called");
	Map<std::wstring, TranslatedText>& map = lang.translateTable;
	map.emplace(L"hbp.title", TranslatedText(L"高血压"));
	map.emplace(L"hbp.confirm.confirm", TranslatedText(L"确认"));
	map.emplace(L"hbp.confirm.cancel", TranslatedText(L"取消"));
	map.emplace(L"hbp.caption.close", TranslatedText(L"\\#ffee0000关闭窗口"));
	map.emplace(L"hbp.caption.maximize", TranslatedText(L"\\#ff4488ee最大化窗口"));
	map.emplace(L"hbp.caption.restore", TranslatedText(L"\\#ff4488ee复原窗口"));
	map.emplace(L"hbp.caption.minimize", TranslatedText(L"\\#ffeeaaaa最小化窗口"));
	map.emplace(L"hbp.float.freshCanvas", TranslatedText(L"\\.ff4488aa\\#ff000000右键以刷新窗口绘制"));
	map.emplace(L"hbp.button.exit", TranslatedText(L"\\#ff44ee66退出"));
	map.emplace(L"hbp.float.exit", TranslatedText(L"\\#ff44ee66退出游戏"));
	map.emplace(L"hbp.confirming.exit", TranslatedText(L"是否\\#ff44ee66退出游戏\\r？"));
	map.emplace(L"hbp.button.settings", TranslatedText(L"\\#ff4488aa设置"));
	map.emplace(L"hbp.float.settings", TranslatedText(L"\\#ff4488aa设置"));
	map.emplace(L"hbp.button.start", TranslatedText(L"\\#ffee0000开始"));
	map.emplace(L"hbp.float.start", TranslatedText(L"\\#ffee0000开始游戏"));
	map.emplace(L"hbp.confirming.start", TranslatedText(L"是否\\#ffee0000退出游戏\\r？"));
}

inline Translator translator = Translator();
