
#pragma once

#include <stdexcept> // 可能舍弃
#include <string>
#include <string_view>
#include "src\using.hpp"

template <typename T> constexpr size_t lengthCharLiteral(const T* const chars) noexcept { return std::char_traits<T>::length(chars); }

[[nodiscard]] inline unsigned int wtouib16(const wchar* string) noexcept {
	unsigned int ret = 0;
	while (*string != L'\0') {
		if (ret & 0xf0000000) return 0xffffffff;
		if (*string >= L'0' && *string <= L'9') {
			ret <<= 4;
			ret += *string - L'0';
		}
		else if (*string >= L'A' && *string <= L'F') {
			ret <<= 4;
			ret += *string - 0x41; // 'A' - 10
		}
		else if (*string >= L'a' && *string <= L'f') {
			ret <<= 4;
			ret += *string - 0x61; // 'a' - 10
		}
		else return 0xffffffff;
		++string;
	}
	return ret;
}

[[nodiscard]] inline unsigned int wtouib16(const wchar* const string, const unsigned long long length) noexcept {
	unsigned int ret = 0;
	unsigned long long i = 0;
	while (i < length) {
		if (ret & 0xf0000000) return 0xffffffff;
		if (string[i] >= L'0' && string[i] <= L'9') {
			ret <<= 4;
			ret += string[i] - L'0';
		}
		else if (string[i] >= L'A' && string[i] <= L'F') {
			ret <<= 4;
			ret += string[i] - 55; // L'A' - 10
		}
		else if (string[i] >= L'a' && string[i] <= L'f') {
			ret <<= 4;
			ret += string[i] - 87; // L'a' - 10
		}
		else return 0xffffffff;
		++i;
	}
	return ret;
}

static constexpr wchar Table16[17] = L"0123456789ABCDEF";

/**
 * 将数字转换为字符串
 * @param value 要转换的数字
 * @param fills 填充位数。返回的字符串长度一定不小于该值
 * @return String类型
 */

[[nodiscard]] inline String qwtowb16(size_t value, const unsigned int fills = 1) noexcept(false) {
	String ret;
	if (fills >= 16 || value < static_cast<size_t>(1) << fills * 4) {
		ret.assign(fills, L'0');
		for (size_t i = fills - 1; i && value; --i) {
			ret[i] = Table16[value & 0xf];
			value >>= 4;
		}
		if (value) ret[0] = Table16[value & 0xf];
	}
	else {
		size_t i = 64;
		while (i) {
			i -= 4;
			if (!(value >> i & 0xf)) continue;
			ret.push_back(Table16[value >> i & 0xf]);
			break;
		}
		while (i) {
			i -= 4;
			ret.push_back(Table16[value >> i & 0xf]);
		}
		if (ret.empty()) ret = L"0";
	}
	return ret;
}

[[nodiscard]] inline String qwtowb16_nothrow(size_t value, const unsigned int fills = 1) noexcept {
	try { return qwtowb16(value, fills); }
	catch (std::bad_alloc&) { return String(); }
	catch (...) { return L"?"; }
}

inline String ptrtow(const void* value) noexcept(noexcept(qwtowb16(0))) { return qwtowb16(reinterpret_cast<unsigned long long>(value), 16); }

[[nodiscard]] inline String qwtowb10(size_t value, const unsigned int fills = 1) noexcept(false) {
	static constexpr wchar Table10[11] = L"0123456789";
	static constexpr size_t Compare10[20] = {
		1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull,
		100000000ull, 1000000000ull, 10000000000ull, 100000000000ull,
		1000000000000ull, 10000000000000ull, 100000000000000ull,
		1000000000000000ull, 10000000000000000ull, 100000000000000000ull,
		1000000000000000000ull, 10000000000000000000ull
	};
	String ret;
	if (value < Compare10[fills]) {
		ret.assign(fills, L'0');
		for (unsigned int i = fills - 1; i != 0 && value; --i) {
			ret[i] = Table10[value % 10];
			value /= 10;
		}
	}
	else {
		unsigned int i = 0;
		while (i < 19) {
			if (value < Compare10[i]) break;
			++i;
		}
		while (i < 19) {
			ret.push_back(Table10[value / Compare10[i]]);
			value %= Compare10[i];
			++i;
		}
		if (ret.empty()) ret = L"0";
	}
	return ret;
}

[[nodiscard]] inline String qwtowb10_nothrow(size_t value, const unsigned int fills = 1) noexcept {
	try { return qwtowb10(value, fills); }
	catch (std::bad_alloc&) { return String(); }
	catch (...) { return L"?"; }
}

/**
 * @brief AI代写的utf互转器
 * @param utf8 传入窄字符串
 * @return 转换结果宽字符串
 */
[[nodiscard]] inline String u8to16w(const std::string_view& utf8) noexcept(false) {
	String utf16;
	utf16.reserve(utf8.size()); // 预分配空间
	for (size_t i = 0; i < utf8.size();) {
		unsigned int code_point = 0;
		const unsigned char first_byte = static_cast<unsigned char>(utf8[i]);
		if ((first_byte & 0x80) == 0x00) { // 1字节: 0xxxxxxx
			code_point = first_byte & 0x7F;
			i += 1;
		}
		else if ((first_byte & 0xE0) == 0xC0) { // 2字节: 110xxxxx 10xxxxxx
			if (i + 1 >= utf8.size()) throw std::runtime_error("Invalid UTF-8 sequence");
			const unsigned char second_byte = static_cast<unsigned char>(utf8[i + 1]);
			if ((second_byte & 0xC0) != 0x80) throw std::runtime_error("Invalid UTF-8 sequence");
			code_point = (first_byte & 0x1F) << 6 | second_byte & 0x3F;
			i += 2;
		}
		else if ((first_byte & 0xF0) == 0xE0) { // 3字节: 1110xxxx 10xxxxxx 10xxxxxx
			if (i + 2 >= utf8.size()) throw std::runtime_error("Invalid UTF-8 sequence");
			const unsigned char second_byte = static_cast<unsigned char>(utf8[i + 1]);
			const unsigned char third_byte = static_cast<unsigned char>(utf8[i + 2]);
			if ((second_byte & 0xC0) != 0x80 || (third_byte & 0xC0) != 0x80) throw std::runtime_error("Invalid UTF-8 sequence");
			code_point = (first_byte & 0x0F) << 12 | (second_byte & 0x3F) << 6 | third_byte & 0x3F;
			i += 3;
		}
		else if ((first_byte & 0xF8) == 0xF0) { // 4字节: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
			if (i + 3 >= utf8.size()) throw std::runtime_error("Invalid UTF-8 sequence");
			const unsigned char second_byte = static_cast<unsigned char>(utf8[i + 1]);
			const unsigned char third_byte = static_cast<unsigned char>(utf8[i + 2]);
			const unsigned char fourth_byte = static_cast<unsigned char>(utf8[i + 3]);
			if ((second_byte & 0xC0) != 0x80 || (third_byte & 0xC0) != 0x80 || (fourth_byte & 0xC0) != 0x80) throw std::runtime_error("Invalid UTF-8 sequence");
			code_point = (first_byte & 0x07) << 18 | (second_byte & 0x3F) << 12 | (third_byte & 0x3F) << 6 | fourth_byte & 0x3F;
			i += 4;
		}
		else throw std::runtime_error("Invalid UTF-8 sequence");

		if (code_point <= 0xFFFF) utf16.push_back(static_cast<wchar_t>(code_point)); // BMP字符（U+0000到U+FFFF）
		else if (code_point <= 0x10FFFF) { // 补充平面字符（U+10000到U+10FFFF）转换为代理对
			code_point -= 0x10000;
			const unsigned short high_surrogate = static_cast<unsigned short>(code_point >> 10 | 0xD800);
			const unsigned short low_surrogate = static_cast<unsigned short>(code_point & 0x3FF | 0xDC00);
			utf16.push_back(static_cast<wchar_t>(high_surrogate));
			utf16.push_back(static_cast<wchar_t>(low_surrogate));
		}
		else throw std::runtime_error("Invalid Unicode code point");
	}

	return utf16;
}

/**
 * @brief AI代写的utf互转器，粗暴地忽略错误的版本
 * @param utf8 传入窄字符串
 * @return 转换结果宽字符串
 */
[[nodiscard]] inline String u8to16w_nothrow(const std::string_view& utf8) noexcept {
	try { return u8to16w(utf8); }
	catch (...) { return {}; }
}

/**
 * @brief AI代写的utf互转器
 * @param utf16 传入宽字符串
 * @return 转换结果窄字符串
 */
[[nodiscard]] inline std::string u16wto8(const StringView& utf16) noexcept(false) {
	std::string utf8;
	utf8.reserve(utf16.size() * 3); // 预分配空间

	for (size_t i = 0; i < utf16.size();) {
		unsigned int code_point = 0;
		const unsigned short first_word = static_cast<unsigned short>(utf16[i]);

		if (first_word >= 0xD800 && first_word <= 0xDBFF) { // 检查是否为高代理项（0xD800-0xDBFF）
			// 代理对
			if (i + 1 >= utf16.size()) throw std::runtime_error("Invalid UTF-16 sequence (missing low surrogate)");
			const unsigned short second_word = static_cast<unsigned short>(utf16[i + 1]);
			if (second_word < 0xDC00 || second_word > 0xDFFF) // 检查是否为低代理项（0xDC00-0xDFFF）
				throw std::runtime_error("Invalid UTF-16 sequence (invalid low surrogate)");
			code_point = 0x10000 + (static_cast<unsigned int>(first_word & 0x3FF) << 10 | second_word & 0x3FF); // 计算码点
			i += 2;
		}
		else if (first_word >= 0xDC00 && first_word <= 0xDFFF) // 检查是否为低代理项（不应该单独出现）
			throw std::runtime_error("Invalid UTF-16 sequence (lone low surrogate)");
		else { // BMP字符
			code_point = first_word;
			i += 1;
		}

		if (code_point <= 0x7F) // 1字节: 0xxxxxxx
			utf8.push_back(static_cast<char>(code_point));
		else if (code_point <= 0x7FF) { // 2字节: 110xxxxx 10xxxxxx
			utf8.push_back(static_cast<char>(0xC0 | code_point >> 6 & 0x1F));
			utf8.push_back(static_cast<char>(0x80 | code_point & 0x3F));
		}
		else if (code_point <= 0xFFFF) { // 3字节: 1110xxxx 10xxxxxx 10xxxxxx
			utf8.push_back(static_cast<char>(0xE0 | code_point >> 12 & 0x0F));
			utf8.push_back(static_cast<char>(0x80 | code_point >> 6 & 0x3F));
			utf8.push_back(static_cast<char>(0x80 | code_point & 0x3F));
		}
		else if (code_point <= 0x10FFFF) { // 4字节: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
			utf8.push_back(static_cast<char>(0xF0 | code_point >> 18 & 0x07));
			utf8.push_back(static_cast<char>(0x80 | code_point >> 12 & 0x3F));
			utf8.push_back(static_cast<char>(0x80 | code_point >> 6 & 0x3F));
			utf8.push_back(static_cast<char>(0x80 | code_point & 0x3F));
		}
		else throw std::runtime_error("Invalid Unicode code point");
	}

	return utf8;
}

/**
 * @brief AI代写的utf互转器，粗暴地忽略错误的版本
 * @param utf16 传入宽字符串
 * @return 转换结果窄字符串
 */
[[nodiscard]] inline std::string u16wto8_nothrow(const StringView& utf16) noexcept {
	try { return u16wto8(utf16); }
	catch (...) { return {}; }
}

#ifndef FunctionSignature
#define FunctionSignature() (u8to16w_nothrow(__FUNCSIG__))
#endif
