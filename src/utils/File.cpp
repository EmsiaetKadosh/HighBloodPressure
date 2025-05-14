
#include "..\def.h"

#include "exception.h"
#include "File.h"

File& File::open() {
	if (file.is_open()) file.close();
	file.open(path, flags);
	return *this;
}

File& File::close() {
	if (file.is_open()) file.close();
	return *this;
}

File& File::clearContent() {
	file.clear();
	if (file.is_open()) file.close();
	file.open(path, flags | std::ios::trunc);
	return *this;
}

File& File::inputs(const bool value) & noexcept {
	if (value) flags |= std::ios::in;
	else flags &= ~std::ios::in;
	return *this;
}

File& File::outputs(const bool value) & noexcept {
	if (value) flags |= std::ios::out;
	else flags &= ~std::ios::out;
	return *this;
}

File& File::binary(const bool value) & noexcept {
	if (value) flags |= std::ios::binary;
	else flags &= ~std::ios::binary;
	return *this;
}

File& File::append(const bool value) & noexcept {
	if (value) flags |= std::ios::app;
	else flags &= ~std::ios::app;
	return *this;
}

File& File::truncate(const bool value) & noexcept {
	if (value) flags |= std::ios::trunc;
	else flags &= ~std::ios::trunc;
	return *this;
}

DataInteger::DataInteger(String&& name, const String& value) : Data(std::move(name), DataType::Integer) {
	const String& str = value;
	errno = 0;
	wchar* end;
	if (str.starts_with(L'-')) {
		// 负数
		data = std::wcstoull(str.c_str() + 1, &end, 0);
		data = -data;
	} else data = std::wcstoull(str.c_str(), &end, 0);
	if (end == &str.back() + 1) return;
	if (errno == ERANGE) Logger.error(Logger.of(L"int out of range: \"", end, L"\" in \"", str, L"\""));
	Logger.error(Logger.of(L"Failed to parse int: \"", str, L"\""));
}

DataDouble::DataDouble(String&& name, const String& value) : Data(std::move(name), DataType::Double) {
	const String& str = value;
	errno = 0;
	wchar* end;
	this->data = wcstod(str.c_str(), &end);
	if (end == &str.back() + 1) return;
	if (errno == ERANGE) Logger.error(Logger.of(L"double out of range: \"", end, L"\" in \"", str, L"\" @", name));
	Logger.error(Logger.of(L"Failed to parse int: \"", str, L"\" @", name));
}

DataString::DataString(String&& name, String&& value) : Data(std::move(name), DataType::String), value(value) {}

DataBoolean::DataBoolean(String&& name, const String& value) : Data(std::move(name), DataType::Boolean) {
	if (value == L"true") booleanIntegerSigned = true;
	else if (value == L"false") booleanIntegerSigned = false;
	else booleanIntegerSigned = false, Logger.error(Logger.of(L"Unknown boolean value:", name, value));
}

DataNull::DataNull(String&& name) : Data(std::move(name), DataType::Null) {}
DataList::DataList(String&& name, List<Data*>&& list) : Data(std::move(name), DataType::List), list(std::move(list)) {}
DataObject::DataObject(String&& name, Set<Data*, LessData>&& object) : Data(std::move(name), DataType::List), object(std::move(object)) {}

int DataLoader::parseString(int& line, String& string) {
	while (!file.file.eof()) {
		wchar c = file.file.get();
		if (c == L'\"') Success();
		if (c == L'\\') { c = file.file.get(); }
	}
	errorInfo = L"Error: String never ends. EOF comes before a '\"'";
	Failed();
}

int DataLoader::loadUntil(int& line, wchar at) {
	String name = {};
	String val = {};
	enum : unsigned char { identifier, eq, value, end } status = identifier;
	while (!file.file.eof()) {
		const wchar c = file.file.get();
		if (c == L'\n') ++line;
		switch (status) {
			case identifier:
				if (!isspace(c)) name.append(1, c);
				else if (!name.empty()) status = eq;
				break;
			case eq:
				if (c == L'=') status = value;
				else if (!isspace(c)) {
					errorInfo = L"Error: expected '=' after identifier at line " + std::to_wstring(line);
					Failed();
				}
				break;
			case value:
				if (!isspace(c)) { if (c == L'\"') {} }
				break;
			case end:
				break;
		}
	}
	return 0;
}

File FileAccessor::getAccess(const String& path) {
	namespace fs = std::filesystem;
	using Path = fs::path;
	const Path original = path;
	const Path p = original.parent_path();
	if (!fs::exists(p)) {
		create_directories(p);
		Logger.info(L"Created directory " + p.wstring());
	}
	return File(p);
}

String StringParser::parse(const String& input) {
	if (input.empty()) return L"";
	// 检查是否是原始字符串
	if (input.size() >= 2 && input[0] == L'R' || input[0] == L'r' && input[1] == L'"') return parseRawString(input);
	return parseRegularString(input);
}

String StringParser::parseRawString(const String& input) noexcept {
	const size_t delimiterEnd = input.find(L'(', 2);
	if (delimiterEnd == String::npos) return Logger.error(L"Malformed raw string literal - missing '('"), L"";
	const String delimiter = input.substr(2, delimiterEnd - 2);
	const String closingDelimiter = L")" + delimiter + L"\"";
	const size_t contentStart = delimiterEnd + 1;
	const size_t contentEnd = input.rfind(closingDelimiter);
	if (contentEnd == String::npos) return Logger.error(L"Malformed raw string literal - missing closing delimiter"), L"";
	return input.substr(contentStart, contentEnd - contentStart);
}

String StringParser::parseRegularString(const String& input) noexcept {
	if (input.size() < 2 || input.front() != L'"' || input.back() != L'"') throw std::invalid_argument("Invalid string literal - must be enclosed in double quotes");
	String result;
	result.reserve(input.size() - 2); // Reserve space for performance

	for (size_t i = 1; i < input.size() - 1; ++i)
		if (input[i] != L'\\') result += input[i];
		else {
			if (i + 1 >= input.size() - 1) return Logger.error(L"Incomplete escape sequence at end of string"), result;
			const wchar escaped = input[++i];
			if (auto it = escapeSequences.find(escaped); it != escapeSequences.end()) result += it->second;
			else if (escaped == L'x') result += parseHexEscape(input, i);
			else if (escaped == L'u') result += parseUnicodeEscape(input, i, 4);
			else if (escaped == L'U') result += parseUnicodeEscape(input, i, 8);
			else if (std::isdigit(escaped)) result += parseOctalEscape(input, i);
			else result += escaped;
		}

	return result;
}

wchar StringParser::parseHexEscape(const String& input, size_t& pos) noexcept {
	if (pos + 2 >= input.size() - 1) return Logger.error(Logger.of(L"Incomplete hex escape sequence:", input.substr(pos, 2))), L'?';
	const String hexStr = input.substr(pos + 1, 2);
	pos += 2;
	try { return static_cast<wchar>(std::stoi(hexStr, nullptr, 16)); } catch (...) {
		Logger.error(Logger.of(L"Invalid hex escape sequence:", hexStr));
		return L'?';
	}
}

String StringParser::parseUnicodeEscape(const String& input, size_t& pos, const int length) noexcept {
	if (pos + length >= input.size() - 1) return Logger.error(Logger.of(L"Incomplete Unicode escape sequence:", input.substr(pos + 1))), L"?";
	const String hexStr = input.substr(pos + 1, length);
	pos += length;
	try {
		const unsigned long code = std::stoul(hexStr, nullptr, 16);
		return codePointToUTF8(code);
	} catch (...) { return Logger.error(Logger.of(L"Invalid Unicode escape sequence:", hexStr)), L"?"; }
}

wchar StringParser::parseOctalEscape(const String& input, size_t& pos) noexcept {
	size_t end = pos;
	while (end < input.size() - 1 && end - pos < 3 && input[end + 1] >= L'0' && input[end + 1] <= L'7') ++end;

	const String octStr = input.substr(pos, end - pos + 1);
	pos = end;

	try { return static_cast<wchar>(std::stoi(octStr, nullptr, 8)); } catch (...) {
		Logger.error(Logger.of(L"Invalid octal escape sequence:", octStr));
		return L'?';
	}
}

String StringParser::codePointToUTF8(const unsigned long codePoint) noexcept {
	String result;

	if (codePoint <= 0x7F) result += static_cast<wchar>(codePoint);
	else if (codePoint <= 0x7FF) {
		result += static_cast<wchar>(0xC0 | codePoint >> 6 & 0x1F);
		result += static_cast<wchar>(0x80 | codePoint & 0x3F);
	} else if (codePoint <= 0xFFFF) {
		result += static_cast<wchar>(0xE0 | codePoint >> 12 & 0x0F);
		result += static_cast<wchar>(0x80 | codePoint >> 6 & 0x3F);
		result += static_cast<wchar>(0x80 | codePoint & 0x3F);
	} else if (codePoint <= 0x10FFFF) {
		result += static_cast<wchar>(0xF0 | codePoint >> 18 & 0x07);
		result += static_cast<wchar>(0x80 | codePoint >> 12 & 0x3F);
		result += static_cast<wchar>(0x80 | codePoint >> 6 & 0x3F);
		result += static_cast<wchar>(0x80 | codePoint & 0x3F);
	} else Logger.error(Logger.of(L"Invalid Unicode code point:", codePoint));

	return result;
}

inline const std::unordered_map<wchar, wchar> StringParser::escapeSequences = {
	{ L'\'', L'\'' },
	{ L'\"', L'\"' },
	{ L'?', L'\?' },
	{ L'\\', L'\\' },
	{ L'a', L'\a' },
	{ L'b', L'\b' },
	{ L'f', L'\f' },
	{ L'n', L'\n' },
	{ L'r', L'\r' },
	{ L't', L'\t' },
	{ L'v', L'\v' }
};
