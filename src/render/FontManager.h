//
// Created by EmsiaetKadosh on 25-6-18.
//

#pragma once


#include "..\utils\exception.h"
#include "..\utils\Chars.h"
#include "..\interact\InteractManager.h"

class [[carlbeks::predecl, carlbeks::defineat("renderer.h")]] GdiRenderer;

using FontStyle = int;
using FontID = unsigned short;

struct StringConfig {
	String text;
	unsigned int color = -1;
	unsigned int background = -1;
	/**
	 * 位意义
	 * 1 - bold
	 * 2 - italic
	 * 4 - underline
	 * 8 - strikeThrough
	 * 16 - default bg
	 * 32 - default clr
	 */
	FontStyle style = 0;
	FontID idFont = 0;

	StringConfig() = default;

	void reset() noexcept {
		idFont = 0;
		color = -1;
		background = -1;
		style = 0;
	}

	[[nodiscard]] bool isBold() const noexcept { return style & 1; }
	[[nodiscard]] bool isItalic() const noexcept { return style & 2; }
	[[nodiscard]] bool isUnderline() const noexcept { return style & 4; }
	[[nodiscard]] bool isStrikeThrough() const noexcept { return style & 8; }
	[[nodiscard]] bool isDefaultBackground() const noexcept { return (style & 16) == 0; }
	[[nodiscard]] bool isDefaultColor() const noexcept { return (style & 32) == 0; }
	void setBold(const bool value) noexcept { style = value ? style | 1 : style & ~1; }
	void setItalic(const bool value) noexcept { style = value ? style | 2 : style & ~2; }
	void setUnderline(const bool value) noexcept { style = value ? style | 4 : style & ~4; }
	void setStrikeThrough(const bool value) noexcept { style = value ? style | 8 : style & ~8; }
	void useDefaultBackground(const bool value) noexcept { style = value ? style & ~16 : style | 16; }
	void useDefaultColor(const bool value) noexcept { style = value ? style & ~32 : style | 32; }

	[[nodiscard]] StringConfig copyConfig() const noexcept {
		StringConfig ret;
		ret.idFont = idFont;
		ret.color = color;
		ret.background = background;
		ret.style = style;
		return ret;
	}

	[[nodiscard]] StringConfig copy() const noexcept {
		StringConfig ret;
		ret.idFont = idFont;
		ret.text = text;
		ret.idFont = idFont;
		ret.color = color;
		ret.background = background;
		ret.style = style;
		return ret;
	}

	[[nodiscard]] String toString() const noexcept {
		String ret;
		ret.append(L"#");
		ret.append(isDefaultColor() ? L"~~" : qwtowb16(color, 8));
		ret.append(L".");
		ret.append(isDefaultBackground() ? L"~~" : qwtowb16(background, 8));
		ret.append(L",F");
		ret.append(std::to_wstring(idFont));
		ret.append(L",");
		if (isBold()) ret.append(L"b");
		if (isItalic()) ret.append(L"i");
		if (isUnderline()) ret.append(L"u");
		if (isStrikeThrough()) ret.append(L"s");
		ret.append(L":");
		ret.append(text);
		return ret;
	}
};

class IFonts {
protected:
	friend class RenderableString;

	const String name;
	double yOffset;
	double heightModifier;
	long height;
	long escapement;
	long orientation;
	long yOffsetPx;
	const FontID id;
	bool adaptAllSize = false;

	IFonts(const FontID id, const String& name, const double heightModifier, const double yOffset, const long escapement, const long orientation, const bool adaptAllSize) : name { name }, yOffset(yOffset), heightModifier(heightModifier), height(static_cast<long>(interactSettings.actual.fontHeight * heightModifier)), escapement(escapement), orientation(orientation), yOffsetPx(static_cast<long>(yOffset * height)), id(id), adaptAllSize(adaptAllSize) {}
	IFonts(const FontID id, String&& name, const double heightModifier, const double yOffset, const long escapement, const long orientation, const bool adaptAllSize) : name { std::move(name) }, yOffset(yOffset), heightModifier(heightModifier), height(static_cast<long>(interactSettings.actual.fontHeight * heightModifier)), escapement(escapement), orientation(orientation), yOffsetPx(static_cast<long>(yOffset * height)), id(id), adaptAllSize(adaptAllSize) {}

	[[nodiscard]] virtual int getWidth(const StringConfig& config) const noexcept = 0;

public:
	IFonts(const IFonts&) = default;
	IFonts(IFonts&&) = default;
	virtual ~IFonts() = default;
	virtual void draw(const RenderableString& text, int x, int y, unsigned int color = 0xffeeeeee) const noexcept = 0;
	virtual void drawCenter(const RenderableString& text, int x, int y, int w, int h, unsigned int color = 0xffeeeeee) const noexcept = 0;
	[[nodiscard]] int getHeight() const noexcept { return height; }
	[[nodiscard]] int getEscapement() const noexcept { return escapement; }
	[[nodiscard]] int getOrientation() const noexcept { return orientation; }
	[[nodiscard]] FontID getID() const noexcept { return id; }
};

/**
 * @note 默认字体编号：
 * 1 - 标题栏字体，HBP
 * 2 - 默认字体，HBP
 * 3 - 调试字体，Jetbrains Mono
 * 4 - 语言字体（中文），STSong
 * 5 - 默认字体
 */
class IFontManager {
	friend class IRenderer;

protected:
	IRenderer* renderer;

public:
	IFontManager(IRenderer* const renderer) : renderer(renderer) { initializerChecker.requiredModule(L"FontManager", L"Renderer").registerModule(L"FontManager"); }
	IFontManager(const IFontManager&) = delete;
	IFontManager(IFontManager&&) = delete;
	IFontManager& operator=(const IFontManager&) = delete;
	IFontManager& operator=(IFontManager&&) = delete;
	virtual ~IFontManager() = default;

	virtual void finalize() = 0;
	virtual void resize(int width, int height) = 0;
	virtual IFonts& newFont(const String& name, double heightModifier, double yOffset, bool adaptAllSize, long escapement, long orientation) = 0;
	virtual IFonts& newFont(String&& name, double heightModifier, double yOffset, bool adaptAllSize, long escapement, long orientation) = 0;
	[[nodiscard]] virtual IFonts& getDefault() const noexcept = 0;
	[[nodiscard]] virtual IFonts& get(FontID id) const noexcept = 0;
};
