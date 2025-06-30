//
// Created by EmsiaetKadosh on 25-1-18.
//

#include "IText.h"

#include "..\def.h"
#include "..\render\Renderer.h"

int RenderableString::getHeight() const noexcept {
	int height = 0;
	for (const StringConfig& config : configs) if (const IFonts& font = fontManager.get(config.idFont); font.getHeight() > height) height = font.getHeight();
	return height;
}

int RenderableString::getWidth(const FontID defaultID) const noexcept {
	int width = 0;
	for (const StringConfig& config : configs) width += fontManager.get(config.idFont ? config.idFont : defaultID).getWidth(config);
	return width;
}

int RenderableString::getWidth(RenderConfig* renderConfigs, const FontID defaultID) const noexcept {
	int width = 0;
	int i = 0;
	for (const StringConfig& config : configs) {
		renderConfigs[i].config = &config;
		const IFonts& font = fontManager.get(config.idFont ? config.idFont : defaultID);
		renderConfigs[i].font = &font;
		width += renderConfigs[i].width = font.getWidth(config);
		++i;
	}
	return width;
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
