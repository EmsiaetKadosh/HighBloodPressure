//
// Created by EmsiaetKadosh on 25-1-17.
//

#pragma once

#include "..\def.h"

[[nodiscard]] inline unsigned int wtouib16(const wchar* string) noexcept {
	unsigned int ret = 0;
	while (*string != L'\0') {
		if (ret & 0xf0000000) return 0xffffffff;
		if (*string >= L'0' && *string <= L'9') {
			ret <<= 4;
			ret += *string - L'0';
		} else if (*string >= L'A' && *string <= L'F') {
			ret <<= 4;
			ret += *string - 0x41; // 'A' - 10
		} else if (*string >= L'a' && *string <= L'f') {
			ret <<= 4;
			ret += *string - 0x61; // 'a' - 10
		} else return 0xffffffff;
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
		} else if (string[i] >= L'A' && string[i] <= L'F') {
			ret <<= 4;
			ret += string[i] - 55; // L'A' - 10
		} else if (string[i] >= L'a' && string[i] <= L'f') {
			ret <<= 4;
			ret += string[i] - 87; // L'a' - 10
		} else return 0xffffffff;
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

[[nodiscard]] inline String qwtowb16(QWORD value, const unsigned int fills = 1) noexcept {
	String ret;
	if (fills >= 16 || value < static_cast<QWORD>(1) << fills * 4) {
		ret.assign(fills, L'0');
		for (QWORD i = fills - 1; i && value; --i) {
			ret[i] = Table16[value & 0xf];
			value >>= 4;
		}
		if (value) ret[0] = Table16[value & 0xf];
	}
	else {
		QWORD i = 64;
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

[[nodiscard]] inline String qwtowb10(QWORD value, const unsigned int fills = 1) noexcept {
	static constexpr wchar Table10[11] = L"0123456789";
	static constexpr QWORD Compare10[20] = {
		0ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull,
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
	} else {
		unsigned int i = 0;
		while (i < 19) {
			if (value >= Compare10[i]) break;
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


[[nodiscard]] inline String atow(const char* chars) {
	const int c = MultiByteToWideChar(CP_ACP, 0, chars, -1, nullptr, 0);
	wchar* e = new wchar[c];
	MultiByteToWideChar(CP_ACP, 0, chars, -1, e, c);
	String ret(e);
	delete[] e;
	return ret;
}

[[nodiscard]] inline std::string wtoa(const wchar* wchars) {
	const int c = WideCharToMultiByte(CP_ACP, 0, wchars, -1, nullptr, 0, nullptr, nullptr);
	char* e = new char[c];
	WideCharToMultiByte(CP_ACP, 0, wchars, -1, e, c, nullptr, nullptr);
	std::string ret = e;
	delete[] e;
	return ret;
}
