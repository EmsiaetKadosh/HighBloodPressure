//
// Created by EmsiaetKadosh on 25-3-7.
//

#pragma once

#include "..\def.h"

class File final {
public:
	String path{};
	std::wfstream file{};
	int flags = std::ios::in | std::ios::out | std::ios::binary;

	File(const String& path) : path(path) {}
	File(const File&) = delete;
	File(File&&) = delete;
	~File() = default;
	File& operator=(const File&) = delete;
	File& operator=(File&&) = delete;

	File& open();
	File& close();
	File& clearContent();
	File& inputs(bool value = true) & noexcept;
	File&& inputs(const bool value = true) && noexcept { return std::move(inputs(value)); }
	File& outputs(bool value = true) & noexcept;
	File&& outputs(const bool value = true) && noexcept { return std::move(outputs(value)); }
	File& binary(bool value = true) & noexcept;
	File&& binary(const bool value = true) && noexcept { return std::move(binary(value)); }
	File& append(bool value = true) & noexcept;
	File&& append(const bool value = true) && noexcept { return std::move(append(value)); }
	File& truncate(bool value = true) & noexcept;
	File&& truncate(const bool value = true) && noexcept { return std::move(truncate(value)); }

	template <typename T>
	File& operator<<(T&& value) {
		if (file.is_open()) file << std::forward<T>(value);
		return *this;
	}

	File& operator<<(decltype(std::endl<wchar, std::char_traits<wchar>>)& value) {
		if (file.is_open()) file << value;
		return *this;
	}

	template <typename T>
	File& operator>>(T&& value) {
		if (file.is_open()) file >> std::forward<T>(value);
		return *this;
	}
};

class [[carlbeks::predecl]] DataInteger;
class [[carlbeks::predecl]] DataDouble;
class [[carlbeks::predecl]] DataString;
class [[carlbeks::predecl]] DataBoolean;
class [[carlbeks::predecl]] DataNull;
class [[carlbeks::predecl]] DataList;
class [[carlbeks::predecl]] DataObject;

class Data {
public:
	friend struct [[carlbeks::predecl]] LessData;

	enum class DataType : unsigned char {
		Integer, Double, String, Boolean, Null, List, Object
	};

	enum class DataStatus : unsigned char {
		Valid, ParseError, Null
	};

protected:
	String name;
	DataType type;
	DataStatus status = DataStatus::Valid;
	bool booleanIntegerSigned = false; // 两个作用，在Boolean中和Integer中
	unsigned char unused = 0;
	unsigned int listLength = 0;

	Data(String&& name, const DataType type): name(std::move(name)), type(type) {}
	virtual ~Data() = default;

public:
	DataType getType() const noexcept { return type; }
	DataStatus getStatus() const noexcept { return status; }
};

struct LessData {
	using is_transparent = void;
	[[nodiscard]] bool operator()(const Data* left, const Data* right) const noexcept { return left->name < right->name; }
	[[nodiscard]] bool operator()(const Data* left, const String& right) const noexcept { return left->name < right; }
	[[nodiscard]] bool operator()(const String& left, const Data* right) const noexcept { return left < right->name; }
};

class DataInteger final : public Data {
	QWORD data;

public:
	DataInteger(String&& name, const String& value);
};

class DataDouble final : public Data {
	double data;

public:
	DataDouble(String&& name, const String& value);
};

class DataString final : public Data {
	String value;

public:
	DataString(String&& name, String&& value);
};

class DataBoolean final : public Data {
public:
	DataBoolean(String&& name, const String& value);
};

class DataNull final : public Data {
public:
	DataNull(String&& name);
};

class DataList final : public Data {
	List<Data*> list;

public:
	DataList(String&& name, List<Data*>&& list);
	~DataList() override { for (const Data* ptr : list) delete deallocating(ptr); }
};

class DataObject final : public Data {
	Set<Data*, LessData> object;

public:
	DataObject(String&& name, Set<Data*, LessData>&& object);
	~DataObject() override { for (const Data* ptr : object) delete deallocating(ptr); }
};

class DataLoader {
	File file;

	int parseString(int& line, String& string);
	int loadUntil(int& line, wchar at = 0);

public:
	String errorInfo = {};
	DataLoader(const String& path) : file(path) {}
	DataLoader(const DataLoader&) = delete;
	DataLoader(DataLoader&&) = delete;
	DataLoader& operator=(const DataLoader&) = delete;
	DataLoader& operator=(DataLoader&&) = delete;
	~DataLoader() = default;

	int load() {
		int line = 1;
		return loadUntil(line);
	}
};

class FileAccessor {
public:
	[[nodiscard]] bool exists(const String& path) const { return std::filesystem::exists(path); }

	File getAccess(const String& path);
};

/**
 * 用于分析字符串
 * @note 允许r"()"和R"()"表示原始字符串，括号两侧可以有识别序列
 */
class StringParser {
	static const std::unordered_map<wchar, wchar> escapeSequences;

public:
	static String parse(const String& input);

private:
	static String parseRawString(const String& input) noexcept;
	static String parseRegularString(const String& input) noexcept;
	static wchar parseHexEscape(const String& input, size_t& pos) noexcept;
	static String parseUnicodeEscape(const String& input, size_t& pos, int length) noexcept;
	static wchar parseOctalEscape(const String& input, size_t& pos) noexcept;
	static String codePointToUTF8(unsigned long codePoint) noexcept;
};

inline FileAccessor fileAccessor = FileAccessor();
