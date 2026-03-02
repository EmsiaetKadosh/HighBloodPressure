
#pragma once

#include <vector>
#include <string>
#include "src\utils\chars.hpp"

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

	void reset() noexcept;

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

	[[nodiscard]] StringConfig copyConfig() const noexcept;
	[[nodiscard]] StringConfig copy() const noexcept;
	[[nodiscard]] String toString() const;
	[[nodiscard]] const String& getString() const noexcept;
};

struct RenderConfig {
	const void* font; // TODO(EmsiaetKadosh): class name
	const StringConfig* config;
	int width;
};

class RenderableString {
	Vector<StringConfig> configs;
	using Iterator = Vector<StringConfig>::iterator;
	using ConstIterator = Vector<StringConfig>::const_iterator;

public:
	RenderableString() noexcept = default;
	RenderableString(const String& string) noexcept : RenderableString(string.c_str(), string.length()) {}
	RenderableString(String&& string) noexcept : RenderableString(string.c_str(), string.length()) {}
	RenderableString(const wchar* string, size_t length = static_cast<size_t>(-1)) noexcept;
	RenderableString(String&& string, nullptr_t) noexcept; // 采用raw字符串而不解析
	RenderableString(const RenderableString&) = default;
	RenderableString(RenderableString&&) noexcept = default;
	~RenderableString() noexcept = default;

	[[nodiscard]] String toString() const;
	[[nodiscard]] String getString() const;

	RenderableString& append(const RenderableString& other);
	RenderableString& append(const String& other);
	[[nodiscard]] int getHeight() const noexcept;
	[[nodiscard]] int getWidth(FontID defaultID = 0) const noexcept;
	[[nodiscard]] int getWidth(RenderConfig* renderConfigs, FontID defaultID) const noexcept;

private:
	void parseAppend(const wchar* string);
	void parseAppend(const wchar* string, size_t length);
};
