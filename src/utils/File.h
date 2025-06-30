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

class FileAccessor {
public:
	[[nodiscard]] bool exists(const String& path) const { return std::filesystem::exists(path); }

	File getAccess(const String& path);
};

inline FileAccessor fileAccessor = FileAccessor();
