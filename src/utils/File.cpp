
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

/*

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

String DataLoader::parseString(std::wistream& in) {
	String result;
	wchar ch;
	if (!(in >> std::ws >> ch)) {
		Logger.error(L"Unexpected end of input");
		return result;
	}
	if (ch == L'R' || ch == L'r') {
		wchar next;
		if (!(in >> next)) {
			Logger.error(L"Unexpected end of input after R/r");
			in.putback(ch);
			return result;
		}
		if (next == L'"') return parseRawString(in, L"");
		in.putback(next);
		in.putback(ch);
	}
	if (ch == L'"') return parseRegularString(in);
	Logger.error(L"Expected string literal");
	in.putback(ch);
	return result;
}

String DataLoader::parseRegularString(std::wistream& in) {
	String result;
	wchar ch;
	bool escape = false;
	while (in.get(ch))
		if (escape) {
			escape = false;
			switch (ch) {
				case L'n':
					result += L'\n';
					break;
				case L't':
					result += L'\t';
					break;
				case L'v':
					result += L'\v';
					break;
				case L'b':
					result += L'\b';
					break;
				case L'r':
					result += L'\r';
					break;
				case L'f':
					result += L'\f';
					break;
				case L'a':
					result += L'\a';
					break;
				case L'\\':
					result += L'\\';
					break;
				case L'\'':
					result += L'\'';
					break;
				case L'"':
					result += L'"';
					break;
				case L'?':
					result += L'\?';
					break;
				case L'x': { // hexadecimal escape
					String hex;
					while (in.get(ch) && isxdigit(ch)) { hex += ch; }
					if (!hex.empty()) {
						in.putback(ch);
						try {
							int value = std::stoi(hex, nullptr, 16);
							result += static_cast<wchar_t>(value);
						} catch (...) { Logger.error(L"Invalid hexadecimal escape sequence"); }
					} else Logger.error(L"\\x used with no following hex digits");
					break;
				}
				case L'0':
				case L'1':
				case L'2':
				case L'3':
				case L'4':
				case L'5':
				case L'6':
				case L'7': { // octal escape
					String octal;
					octal += ch;
					// Read up to 2 more octal digits
					for (int i = 0; i < 2 && in.get(ch) && ch >= '0' && ch <= '7'; ++i) { octal += ch; }
					in.putback(ch);
					try {
						int value = std::stoi(octal, nullptr, 8);
						result += static_cast<wchar_t>(value);
					} catch (...) { Logger.error(L"Invalid octal escape sequence"); }
					break;
				}
				default:
					Logger.error(String(L"Unknown escape sequence: \\") + ch);
					result += ch;
					break;
			}
		} else if (ch == L'\\') escape = true;
		else if (ch == L'"') return result; // End of string
		else result += ch;
	Logger.error(L"Unterminated string literal");
	return result;
}

String DataLoader::parseRawString(std::wistream& in, const String& delimiter) {
	String result;
	wchar_t ch;
	String currentDelim;
	if (delimiter.empty()) {
		while (in.get(ch) && ch != L'(') currentDelim += ch;
		if (ch != L'(') {
			Logger.error(L"Missing '(' in raw string literal");
			return result;
		}
	} else currentDelim = delimiter;
	const String endSequence = L")" + currentDelim + L"\"";
	size_t matchPos = 0;
	while (in.get(ch)) {
		if (ch == endSequence[matchPos]) {
			matchPos++;
			if (matchPos == endSequence.size()) {
				result.erase(result.size() - endSequence.size() + 1);
				return result;
			}
		} else {
			if (matchPos > 0) {
				result.append(endSequence.substr(0, matchPos));
				matchPos = 0;
			}
		}
		result += ch;
	}
	Logger.error(L"Unterminated raw string literal");
	return result;
}

*/

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
