//
// Created by EmsiaetKadosh on 25-1-16.
//

#pragma once


#include "..\def.h"
#include "Chars.h"
#include "exception.h"
#include "..\interact\InteractManager.h"

class LiteralText;
class Translator;
// class Font;
class FontManager;

using FontStyle = int;
using FontID = unsigned short;

class RenderableString {
	friend class LiteralText;
	friend class Font;

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
			ret.append(isDefaultColor() ? L"~~" : uitowb16(color));
			ret.append(L".");
			ret.append(isDefaultBackground() ? L"~~" : uitowb16(background));
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

	struct RenderConfig {
		const Font* font;
		const StringConfig* config;
		int width;
	};

	List<StringConfig> configs;
	using Iterator = List<StringConfig>::iterator;
	using ConstIterator = List<StringConfig>::const_iterator;

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
		while (iterator != other.configs.cend()) configs.emplace_back(*iterator);
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

class Font {
public:
	Function<void(int width, int height)> resize;

private:
	friend class FontManager;
	friend class RenderableString;
	const String name;
	mutable Map<FontStyle, HFONT> fonts{};
	double yOffset;
	double heightModifier;
	long height;
	long escapement;
	long orientation;
	long yOffsetPx;
	const FontID id;
	bool adaptAllSize = false;

	Font(const FontID id, const String& name, const double heightModifier, const double yOffset, const long escapement, const long orientation, const bool adaptAllSize) : name{name}, yOffset(yOffset), heightModifier(heightModifier), height(static_cast<long>(interactSettings.actual.fontHeight * heightModifier)), escapement(escapement), orientation(orientation), yOffsetPx(static_cast<long>(yOffset * height)), id(id), adaptAllSize(adaptAllSize) {}

	Font(const FontID id, String&& name, const double heightModifier, const double yOffset, const long escapement, const long orientation, const bool adaptAllSize) : name{std::move(name)}, yOffset(yOffset), heightModifier(heightModifier), height(static_cast<long>(interactSettings.actual.fontHeight * heightModifier)), escapement(escapement), orientation(orientation), yOffsetPx(static_cast<long>(yOffset * height)), id(id), adaptAllSize(adaptAllSize) {}

public:
	Font(const Font&) = default;
	Font(Font&&) = default;
	~Font() {
		// 此处Font的回收已经到结束阶段，GDI应该已经收回了资源，不能在手动释放了
		if (!fonts.empty()) Logger.warn(L"Font is not successfully cleared when ~Font() called: " + name);
	}

private:
	HFONT tryCreate(const RenderableString::StringConfig& config) const {
		if (const auto iter = fonts.find(config.style); iter != fonts.end()) return iter->second;
		LOGFONTW f{
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
			.lfFaceName{}
		};
		memcpy(f.lfFaceName, name.c_str(), 64);
		HFONT fnt = CreateFontIndirectW(&f);
		fonts.emplace(std::make_pair(config.style, fnt));
		return fnt;
	}

	int getWidth(const RenderableString::StringConfig& config) const;
	int drawSingle(const RenderableString::StringConfig& config, int x, int y, unsigned int defaultColor) const;
	void drawDirect(const RenderableString::StringConfig& config, int x, int y, unsigned int defaultColor) const;

	void clear() const;

public:
	void draw(const RenderableString& text, int x, int y, unsigned int color = 0xffeeeeee) const;
	void drawCenter(const RenderableString& text, int x, int y, int w, int h, unsigned int color = 0xffeeeeee) const;

	[[nodiscard]] int getHeight() const noexcept { return height; }
	[[nodiscard]] int getEscapement() const noexcept { return escapement; }
	[[nodiscard]] int getOrientation() const noexcept { return orientation; }
	[[nodiscard]] FontID getID() const noexcept { return id; }
};

class FontManager {
	Map<FontID, Font> fonts;
	Font *defaultFont, *captionFont;
	FontID assigned = 0;
	using IterFonts = Map<FontID, Font>::const_iterator;

public:
	FontManager() {
		captionFont = &newFont(L"Microsoft YaHei UI Light");
		defaultFont = &newFont(L"Microsoft YaHei UI Light");
		captionFont->height = interactSettings.actual.captionHeight >> 1;
		captionFont->resize = [this](int, int) {
			if (interactSettings.actual.captionHeight != captionFont->height) {
				captionFont->height = interactSettings.actual.captionHeight >> 1;
				captionFont->clear();
			}
		};
		newFont(L"Jetbrains Mono", 1.0, -0.078);
		newFont(L"STSong", 1.0, -0.12);
		newFont(L"Arial", 1.0, -0.13);
		newFont(L"", 1.0, -0.05);
	}

	void finalize() { for (auto& [id, font] : fonts) font.clear(); }

	[[nodiscard]] const Font& get(const FontID id) const noexcept {
		const IterFonts iter = fonts.find(id);
		if (iter == fonts.cend()) return *defaultFont;
		return iter->second;
	}

	const Font& newFont(const String& name, const double heightModifier = 1.0, const double yOffset = 0.0, const bool adaptAllSize = true, const long escapement = 0, const long orientation = 0) noexcept {
		++assigned;
		return fonts.emplace(assigned, std::move(Font(assigned, name, heightModifier, yOffset, adaptAllSize, escapement, orientation))).first->second;
	}

	Font& newFont(String&& name, const double heightModifier = 1.0, const double yOffset = 0.0, const bool adaptAllSize = true, const long escapement = 0, const long orientation = 0) {
		++assigned;
		return fonts.emplace(assigned, std::move(Font(assigned, std::move(name), heightModifier, yOffset, adaptAllSize, escapement, orientation))).first->second;
	}

	[[nodiscard]] Font& getDefault() const noexcept { return *defaultFont; }

	void resize(const int width, const int height) {
		for (auto& [_, font] : fonts)
			if (font.resize) font.resize(width, height);
			else {
				font.height = static_cast<long>(interactSettings.actual.fontHeight * font.heightModifier);
				font.yOffsetPx = static_cast<long>(font.height * font.yOffset);
				font.clear();
			}
	}
};

interface IText {
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

	const String& getText() const noexcept override { return string; }

	const RenderableString& getRenderableString() const noexcept override { return renderableString; }
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
inline TranslatedText operator""__translated(const wchar* const text, const QWORD length) noexcept { return TranslatedText(String(text, length)); }

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
extern FontManager fontManager;
