//
// Created by EmsiaetKadosh on 25-1-16.
//

#pragma once

#include "..\def.h"
#include "Chars.h"
#include "..\render\FontManager.h"

class [[carlbeks::predecl, carlbeks::defineat("renderer.h")]] IRenderer;
class [[carlbeks::predecl, carlbeks::defineat("renderer.h")]] GdiRenderer;
class LiteralText;
class Translator;

struct RenderConfig {
	const IFonts* font;
	const StringConfig* config;
	int width;
};

class RenderableString {
	friend class LiteralText;
	friend class IFonts;
	friend class GdiFont;
	friend class DirectFont;

	Vector<StringConfig> configs;
	using Iterator = Vector<StringConfig>::iterator;
	using ConstIterator = Vector<StringConfig>::const_iterator;

public:
	RenderableString(const String& string): RenderableString(string.c_str(), string.length()) {}
	RenderableString(String&& string) : RenderableString(string.c_str(), string.length()) {}

	RenderableString(const wchar* const string, const QWORD length = static_cast<QWORD>(-1)) {
		if (length == -1) parseAppend(string);
		else parseAppend(string, length);
	}

	RenderableString(const RenderableString&) = default;
	RenderableString(RenderableString&&) = default;

	~RenderableString() = default;

	[[nodiscard]] String toString() const noexcept {
		String ret;
		for (const auto& config : configs) {
			ret.append(config.toString());
			ret.append(L";\n");
		}
		return ret;
	}

	RenderableString& append(const RenderableString& other) {
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

	RenderableString& append(const String& other) {
		parseAppend(other.c_str(), other.length());
		return *this;
	}

	[[nodiscard]] int getHeight() const noexcept;
	[[nodiscard]] int getWidth(FontID defaultID = 0) const noexcept;
	int getWidth(RenderConfig* renderConfigs, FontID defaultID) const noexcept;

private:
	void parseAppend(const wchar* string) noexcept {
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

	void parseAppend(const wchar* string, const QWORD length) noexcept {
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
};

inline RenderableString operator""_renderable(const wchar* const text, const QWORD length) noexcept { return RenderableString(String(text, length)); }
// inline const wchar* operator""_mark_renderable(const wchar* const text, const QWORD) noexcept { return text; }

struct IText {
	virtual ~IText() = default;
	[[nodiscard]] virtual const String& getText() const noexcept = 0;
	[[nodiscard]] virtual const RenderableString& getRenderableString() const noexcept = 0;
};

typedef class LiteralText final : public IText {
	const String string;
	mutable RenderableString renderableString;

public:
	LiteralText(const String& string): string(string), renderableString(string) {}
	LiteralText(String&& string): string(std::move(string)), renderableString(this->string) {}
	[[nodiscard]] const String& getText() const noexcept override { return string; }
	[[nodiscard]] const RenderableString& getRenderableString() const noexcept override { return renderableString; }
} TranslatedText;

inline LiteralText operator""_literal(const wchar* const text, const QWORD length) noexcept { return LiteralText(String(text, length)); }

class TranslatableText final : public IText {
	const String idSrc;
	mutable const LiteralText* target = nullptr;
	mutable QWORD langConfig = 0;

public:
	TranslatableText(const String& id) : idSrc(id) {}
	TranslatableText(String&& id) : idSrc(std::move(id)) {}
	TranslatableText(const TranslatableText& other) : idSrc(other.idSrc) {}
	TranslatableText(TranslatableText&& other) noexcept : idSrc(std::move(other.idSrc)) {}
	const String& getID() const noexcept { return idSrc; }
	const String& getText() const noexcept override;
	const RenderableString& getRenderableString() const noexcept override;
	void refreshText() const noexcept;
};

inline TranslatableText operator""_translates(const wchar* const text, const QWORD length) noexcept { return TranslatableText(String(text, length)); }

using LangID = unsigned int;

struct Language {
	Map<String, TranslatedText> translateTable;
	LangID id = 1;
	using IterText = Map<String, TranslatedText>::const_iterator;

	Language(const LangID id) : id(id) {}
	Language(const Language&) = delete;
	Language(Language&&) = default;
	Language& operator=(const Language&) = delete;
	Language& operator=(Language&&) = default;

	String toString() {
		std::wstringstream stream{};
		stream.imbue(std::locale("zh-CN.UTF-8"));
		stream << L"Language ID: " << id << std::endl;
		for (const auto& [key, value] : translateTable) stream << L"  " << key << L": (" << value.getText().length() << L") " << value.getText() << std::endl;
		return stream.str();
	}
};

void languageMakeChinese(Language&);

class Translator {
	Map<String, Language> langMap{};
	List<Language*> langList{};
	TranslatedText nullText{L"\\#FF""EE0000<translator-null>"};
	String lang = L"zh-cn";
	LangID idLangMax = 0;
	int langConfig = 1;
	using IterLangName = Map<String, Language>::const_iterator;
	using IterLang = List<Language>::const_iterator;

public:
	void initialize() {
		Language& chinese = addLang(L"zh-cn");
		languageMakeChinese(chinese);
		useLanguage(L"zh-cn");
	}

	Translator() = default;
	Language& addLang(const String& lang) noexcept { return langMap.insert(std::make_pair(lang, Language(++idLangMax))).first->second; }
	Language& addLang(String&& lang) noexcept { return langMap.insert(std::make_pair(lang, Language(++idLangMax))).first->second; }
	Language& getLang(const String& lang) noexcept { return langMap.at(lang); }

	const Language* useLanguage(const String& lang) noexcept {
		const Map<String, Language>::iterator language = langMap.find(lang);
		if (language == langMap.cend()) return nullptr;
		for (const Language* i : langList) if (i->id == language->second.id) return i;
		langList.push_front(&language->second);
		return &language->second;
	}

	void unuseLanguage(const String& lang) noexcept {
		const Map<String, Language>::iterator language = langMap.find(lang);
		if (language == langMap.cend()) return;
		for (const Language* i : langList)
			if (i->id == language->second.id) {
				langMap.erase(lang);
				break;
			}
	}

	void loadLang();

	[[nodiscard]] const TranslatedText* getText(const String& id) const noexcept {
		for (const Language* language : langList) {
			const Language::IterText iterator = language->translateTable.find(id);
			if (iterator != language->translateTable.cend()) return &iterator->second;
		}
		return &nullText;
	}

	[[nodiscard]] int getConfigVersion() const noexcept { return langConfig; }
};

extern Translator translator;
