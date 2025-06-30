//
// Created by EmsiaetKadosh on 25-6-18.
//

#pragma once

#include "..\FontManager.h"

class GdiFont final : public IFonts {
	[[nodiscard]] HFONT tryCreate(const StringConfig& config) const noexcept {
		if (const auto iter = fonts.find(config.style); iter != fonts.end()) return iter->second;
		LOGFONTW f {
			.lfHeight = height,
			.lfWidth = 0,
			.lfEscapement = escapement,
			.lfOrientation = orientation,
			.lfWeight = config.isBold() ? FW_BOLD : FW_NORMAL,
			.lfItalic = config.isItalic(),
			.lfUnderline = config.isUnderline(),
			.lfStrikeOut = config.isStrikeThrough(),
			.lfCharSet = DEFAULT_CHARSET,
			.lfOutPrecision = OUT_DEFAULT_PRECIS,
			.lfClipPrecision = CLIP_DEFAULT_PRECIS,
			.lfQuality = static_cast<unsigned char>(adaptAllSize ? DEFAULT_QUALITY : PROOF_QUALITY),
			.lfPitchAndFamily = FF_DONTCARE,
			.lfFaceName {}
		};
		memcpy(f.lfFaceName, name.c_str(), 64);
		HFONT fnt = CreateFontIndirectW(&f);
		fonts.emplace(std::make_pair(config.style, fnt));
		return fnt;
	}

protected:
	[[nodiscard]] int getWidth(const StringConfig& config) const noexcept override;
	[[nodiscard]] int drawSingle(const StringConfig& config, int x, int y, unsigned int defaultColor) const noexcept;
	void drawDirect(const StringConfig& config, int x, int y, unsigned int defaultColor) const noexcept;
	void clear() const noexcept;
	friend class GdiFontManager;
	friend class RenderableString;
	mutable Map<FontStyle, HFONT> fonts {};
	GdiRenderer* renderer = nullptr;

	GdiFont(IRenderer*, FontID, const String& name, double heightModifier, double yOffset, long escapement, long orientation, bool adaptAllSize);

	GdiFont(IRenderer*, FontID, String&& name, double heightModifier, double yOffset, long escapement, long orientation, bool adaptAllSize);

public:
	Function<void(int width, int height)> resize;

	~GdiFont() override {
		// 此处Font的回收已经到结束阶段，GDI应该已经收回了资源，不能在手动释放了
		if (!fonts.empty()) Logger.warn(L"Font is not successfully cleared when ~Font() called: " + name);
	}

	void draw(const RenderableString& text, int x, int y, unsigned int color = 0xffeeeeee) const noexcept override;
	void drawCenter(const RenderableString& text, int x, int y, int w, int h, unsigned int color = 0xffeeeeee) const noexcept override;
};

class GdiFontManager final : public IFontManager {
	friend class GdiFont;
	Map<FontID, GdiFont> fonts;
	GdiFont* defaultFont;
	GdiFont* captionFont;
	FontID assigned = 0;
	using IterFonts = Map<FontID, GdiFont>::const_iterator;

public:
	GdiFontManager(IRenderer* const renderer) : IFontManager(renderer) {
		captionFont = &GdiFontManager::newFont(L"Carlbeks-HBP", 1.0, -0.04); // 1
		defaultFont = &GdiFontManager::newFont(L"Carlbeks-HBP", 1.0, -0.04); // 2
		GdiFontManager::newFont(L"Jetbrains Mono", 1.0, -0.078); // 3
		GdiFontManager::newFont(L"STSong", 1.0, -0.12); // 4
		GdiFontManager::newFont(L"", 1.0, -0.05); // 5

		captionFont->height = static_cast<long>(interactSettings.actual.captionHeight * 0.5);
		captionFont->resize = [this](int, int) {
			if (static_cast<long>(interactSettings.actual.captionHeight * 0.5) != captionFont->height) {
				captionFont->height = static_cast<long>(interactSettings.actual.captionHeight * 0.5);
				captionFont->clear();
			}
		};
	}

	void finalize() override { for (auto& [id, font] : fonts) font.clear(); }

	[[nodiscard]] GdiFont& get(const FontID id) const noexcept override {
		const IterFonts iter = fonts.find(id);
		if (iter == fonts.cend()) return *defaultFont;
		return const_cast<GdiFont&>(iter->second);
	}

	GdiFont& newFont(const String& name, const double heightModifier = 1.0, const double yOffset = 0.0, const bool adaptAllSize = true, const long escapement = 0, const long orientation = 0) noexcept override {
		++assigned;
		return fonts.emplace(assigned, std::move(GdiFont(renderer, assigned, name, heightModifier, yOffset, adaptAllSize, escapement, orientation))).first->second;
	}

	GdiFont& newFont(String&& name, const double heightModifier = 1.0, const double yOffset = 0.0, const bool adaptAllSize = true, const long escapement = 0, const long orientation = 0) override {
		++assigned;
		return fonts.emplace(assigned, std::move(GdiFont(renderer, assigned, std::move(name), heightModifier, yOffset, adaptAllSize, escapement, orientation))).first->second;
	}

	[[nodiscard]] GdiFont& getDefault() const noexcept override { return *defaultFont; }

	void resize(const int width, const int height) override {
		for (auto& [_, font] : fonts)
			if (font.resize) font.resize(width, height);
			else {
				font.height = static_cast<long>(interactSettings.actual.fontHeight * font.heightModifier);
				font.yOffsetPx = static_cast<long>(font.height * font.yOffset);
				font.clear();
			}
	}
};
