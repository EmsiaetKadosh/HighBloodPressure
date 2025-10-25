
#include <Windows.h>
#include "src\render\string.hpp"

void StringConfig::reset() noexcept {
	idFont = 0;
	color = -1;
	background = -1;
	style = 0;
}

StringConfig StringConfig::copyConfig() const noexcept {
	StringConfig ret;
	ret.idFont = idFont;
	ret.color = color;
	ret.background = background;
	ret.style = style;
	return ret;
}

StringConfig StringConfig::copy() const noexcept {
	StringConfig ret;
	ret.idFont = idFont;
	ret.text = text;
	ret.idFont = idFont;
	ret.color = color;
	ret.background = background;
	ret.style = style;
	return ret;
}

String StringConfig::toString() const noexcept {
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

String StringConfig::getString() const noexcept { return text; }

RenderableString::RenderableString(const wchar* const string, const size_t length) {
	if (length == -1) parseAppend(string);
	else parseAppend(string, length);
}

String RenderableString::toString() const noexcept {
	String ret;
	for (const auto& config : configs) {
		ret.append(config.toString());
		ret.append(L";\n");
	}
	return ret;
}

String RenderableString::getString() const noexcept {
	String ret;
	for (const auto& config : configs) ret.append(config.getString());
	return ret;
}

RenderableString& RenderableString::append(const String& other) {
	parseAppend(other.c_str(), other.length());
	return *this;
}

RenderableString& RenderableString::append(const RenderableString& other) {
	auto iterator = other.configs.cbegin();
	int flags = 0;
	bool clr = true, bg = true, font = true;
	const unsigned int color = configs.back().color;
	const unsigned int background = configs.back().background;
	const FontID idFont = configs.back().idFont;
	while (iterator != other.configs.cend()) {
		if (flags >= 3) break;
		StringConfig config = iterator->copy();
		if (font) {
			if (config.idFont == idFont || config.idFont != 0) {
				font = false;
				++flags;
			}
			else config.idFont = idFont;
		}
		if (clr) {
			if (config.color == color || config.color != 0xffffffff) {
				clr = false;
				++flags;
			}
			else config.color = color;
		}
		if (bg) {
			if (config.background == background || config.background != 0xffffffff) {
				bg = false;
				++flags;
			}
			else config.background = background;
		}
		configs.push_back(std::move(config));
		++iterator;
	}
	while (iterator != other.configs.cend()) configs.emplace_back(*iterator), ++iterator;
	return *this;
}

void RenderableString::parseAppend(const wchar* string) noexcept {
	StringConfig config;
	if (!configs.empty()) {
		config = configs.back();
		configs.pop_back();
	}
	while (*string != L'\0') {
		const wchar* start = string;
		if (!config.text.empty()) {
			configs.push_back(std::move(config));
			config = config.copyConfig();
		}
		while (*string != L'\0' && *string != '\\') ++string;
		if (*string == L'\0') {
			if (*start != L'\0') config.text.append(start, string);
			configs.push_back(std::move(config));
			return;
		}
		if (start != string) config.text.append(start, string);
		if (*++string == L'\0') {
			config.text.append(start, string);
			configs.push_back(std::move(config));
			return;
		}
		if (!config.text.empty()) {
			configs.push_back(std::move(config));
			config = config.copyConfig();
		}
		switch (*string) {
			case L'\\':
				config.text.append(1, L'\\');
				++string;
				continue;
			case L'#': {
				unsigned long long i = 0;
				while (i < 9) if (string[i++] == L'\0') goto end;
				config.color = wtouib16(++string, 8);
				config.useDefaultColor(false);
				string += 8;
				continue;
			}
			case L'.': {
				unsigned long long i = 0;
				while (i < 9) if (string[i++] == L'\0') goto end;
				config.background = wtouib16(++string, 8);
				config.useDefaultBackground(false);
				string += 8;
				continue;
			}
			case L'F':
			case L'f':
				if (*++string == L'\0') goto end;
				config.idFont = *string;
				break;
			case L'-':
			case L's':
			case L'S':
				config.setStrikeThrough(true);
				break;
			case L'_':
			case L'u':
			case L'U':
				config.setUnderline(true);
				break;
			case L'/':
			case L'i':
			case L'I':
				config.setItalic(true);
				break;
			case L'=':
			case L'b':
			case L'B':
				config.setBold(true);
				break;
			case L'r':
			case L'R':
				if (!config.text.empty()) configs.push_back(std::move(config));
				config = StringConfig();
				break;
			default:
				config.text.append(string - 1, 2);
				break;
		}
		++string;
	}
end:
	configs.push_back(std::move(config));
}

void RenderableString::parseAppend(const wchar* string, const size_t length) noexcept {
	StringConfig config = StringConfig();
	if (!configs.empty()) {
		config = configs.back();
		configs.pop_back();
	}
	const wchar* const end = string + length;
	while (string < end) {
		const wchar* start = string;
		if (!config.text.empty()) {
			configs.push_back(std::move(config));
			config = config.copyConfig();
		}
		while (string < end && *string != '\\') ++string;
		if (string >= end) {
			if (start != end) config.text.append(start, end);
			configs.push_back(std::move(config));
			return;
		}
		if (start != string) config.text.append(start, string);
		if (++string == end) {
			config.text.append(start, end);
			configs.push_back(std::move(config));
			return;
		}
		if (!config.text.empty()) {
			configs.push_back(std::move(config));
			config = config.copyConfig();
		}
		switch (*string) {
			case L'\\':
				config.text.append(1, L'\\');
				++string;
				continue;
			case L'#': {
				if (end - string < 9) break;
				config.color = wtouib16(++string, 8);
				config.useDefaultColor(false);
				string += 8;
				continue;
			}
			case L'.': {
				if (end - string < 9) break;
				config.background = wtouib16(++string, 8);
				config.useDefaultBackground(false);
				string += 8;
				continue;
			}
			case L'F':
			case L'f':
				if (++string == end) break;
				config.idFont = *string;
				break;
			case L'-':
			case L's':
			case L'S':
				config.setStrikeThrough(true);
				break;
			case L'_':
			case L'u':
			case L'U':
				config.setUnderline(true);
				break;
			case L'/':
			case L'i':
			case L'I':
				config.setItalic(true);
				break;
			case L'=':
			case L'b':
			case L'B':
				config.setBold(true);
				break;
			case L'r':
			case L'R':
				if (!config.text.empty()) configs.push_back(std::move(config));
				config = StringConfig();
				break;
			default:
				config.text.append(string - 1, 2);
				break;
		}
		++string;
	}
	configs.push_back(std::move(config));
}
