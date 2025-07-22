//
// Created by EmsiaetKadosh on 25-1-18.
//

#pragma once

#include "..\global.hpp"

#include "..\warnings.h"
#include "..\def.h"
#include "File.h"

class Exception : public std::exception {
	const String* type;
	const std::stacktrace stacktrace;

protected:
	String msg;

	Exception(String&& msg, const String* type) : type(type), stacktrace(std::stacktrace::current(2)), msg(std::move(msg)) {}
	Exception(const String& msg, const String* type) : type(type), stacktrace(std::stacktrace::current(2)), msg(msg) {}

public:
	[[nodiscard]] String getMessage() const noexcept { return msg; }
	[[nodiscard]] const String* getType() const noexcept { return type; }
	[[nodiscard]] const std::stacktrace& getStackTrace() const noexcept { return stacktrace; }
	[[nodiscard]] const char* what() const override { return "Use Exception::getMessage() instead."; }
};

class NullPointerException final : public Exception {
	inline static const String type = L"NullPointerException";

public:
	NullPointerException(String&& msg) : Exception(std::move(msg), &type) {}
	NullPointerException(const String& msg) : Exception(msg, &type) {}
};

class BadAllocationException final : public Exception {
	inline static const String type = L"BadAllocationException";

public:
	BadAllocationException(String&& msg) : Exception(std::move(msg), &type) {}
	BadAllocationException(const String& msg) : Exception(msg, &type) {}
};

class ArrayIndexOutOfBoundException final : public Exception {
	inline static const String type = L"ArrayIndexOutOfBoundException";

public:
	ArrayIndexOutOfBoundException(String&& msg) : Exception(std::move(msg), &type) {}
	ArrayIndexOutOfBoundException(const String& msg) : Exception(msg, &type) {}
};

class InvalidOperationException final : public Exception {
	inline static const String type = L"InvalidOperationException";

public:
	InvalidOperationException(String&& msg) : Exception(std::move(msg), &type) {}
	InvalidOperationException(const String& msg) : Exception(msg, &type) {}
};

class RuntimeException final : public Exception {
	inline static const String type = L"RuntimeException";

public:
	RuntimeException(String&& msg) : Exception(std::move(msg), &type) {}
	RuntimeException(const String& msg) : Exception(msg, &type) {}
};

class ZeroValueException final : public Exception {
	inline static const String type = L"ZeroValueException";

public:
	ZeroValueException(String&& msg) : Exception(std::move(msg), &type) {}
	ZeroValueException(const String& msg) : Exception(msg, &type) {}
};

class NullReferenceException final : public Exception {
	inline static const String type = L"NullReferenceException";

public:
	NullReferenceException(String&& msg) : Exception(std::move(msg), &type) {}
	NullReferenceException(const String& msg) : Exception(msg, &type) {}
};

class LoggableString {
	friend class PublicLogger;
	String str;
	PublicLogger* logger;
	bool notLogged = false;
	LoggableString() = delete;
	LoggableString(const LoggableString&) = delete;
	LoggableString(String&& str, PublicLogger* logger) : str(std::move(str)), logger(logger) {};

public:
	~LoggableString() { if (notLogged) log(); }

	PublicLogger& trace() noexcept;
	PublicLogger& debug() noexcept;
	PublicLogger& log() noexcept;
	PublicLogger& info() noexcept;
	PublicLogger& warn() noexcept;
	PublicLogger& error() noexcept;
	PublicLogger& fatal() noexcept;
};

class PublicLogger final {
	File mainLogger = File(L"log.txt");
	[[nodiscard]] String build(const String& msg, const String& type) const;

	template <typename T, typename... Ts> requires requires(std::wstringstream stream, T& t, Ts&&... ts) {
		stream << std::forward<T>(t);
		(stream << ... << std::forward<Ts>(ts));
	}
	static void prints(std::wstringstream& stream, T&& t, Ts&&... ts) {
		stream << L" " << std::forward<T>(t);
		prints(stream, std::forward<Ts>(ts)...);
	}

	template <typename T> requires requires(std::wstringstream stream, T&& t) { stream << std::forward<T>(t); }
	static void prints(std::wstringstream& stream, T&& t) { stream << L" " << std::forward<T>(t); }

	template <typename T, typename... Ts> requires requires(std::wstringstream stream, T&& t, Ts&&... ts) {
		stream << std::forward<T>(t);
		(stream << ... << std::forward<Ts>(ts));
	}
	static void ofs(std::wstringstream& stream, T&& t, Ts&&... ts) {
		stream << L" " << std::forward<T>(t);
		ofs(stream, std::forward<Ts>(ts)...);
	}

	template <typename T> requires requires(std::wstringstream stream, T&& t) { stream << std::forward<T>(t); }
	static void ofs(std::wstringstream& stream, T&& t) { stream << L" " << std::forward<T>(t); }

	static void atomicAcquire() noexcept;
	static void atomicRelease() noexcept;

public:
	const String name;

	PublicLogger(const String& name);

	PublicLogger& put(const String& msg) noexcept {
		const String& str = L"          " + name + L"        " + msg + L"\n";
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		std::wcout << str;
		atomicRelease();
		mainLogger << str;
		return *this;
	}

	PublicLogger& trace(const String& msg) noexcept {
		const String& str = build(msg, L"[Trace] ");
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		std::wcout << str;
		atomicRelease();
		mainLogger << str;
		return *this;
	}

	PublicLogger& debug(const String& msg) noexcept {
		const String& str = build(msg, L"[Debug] ");
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN);
		std::wcout << str;
		atomicRelease();
		mainLogger << str;
		return *this;
	}

	PublicLogger& log(const String& msg) noexcept {
		const String& str = build(msg, L"[Log]   ");
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		std::wcout << str;
		atomicRelease();
		mainLogger << str;
		return *this;
	}

	PublicLogger& info(const String& msg) noexcept {
		const String& str = build(msg, L"[Info]  ");
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		std::wcout << str;
		atomicRelease();
		mainLogger << str;
		return *this;
	}

	PublicLogger& warn(const String& msg) noexcept {
		const String& str = build(msg, L"[Warn]  ");
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		std::wcout << str;
		atomicRelease();
		mainLogger << str;
		return *this;
	}

	PublicLogger& error(const String& msg) noexcept {
		const String& str = build(msg, L"[Error] ");
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
		std::wcout << str;
		atomicRelease();
		mainLogger << str;
		return *this;
	}

	PublicLogger& fatal(const String& msg) noexcept {
		const String& str = build(msg, L"[FATAL] ");
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_RED);
		std::wcout << str;
		atomicRelease();
		mainLogger << str;
		return *this;
	}

	template <typename T> requires requires(T t) { std::wcout << t; }
	PublicLogger& print(T&& msg) noexcept {
		std::wstringstream stream = {};
		stream << L"          " << name << L"        " << msg << std::endl;
		String str = stream.str();
		mainLogger << str;
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE);
		std::wcout << str;
		atomicRelease();
		return *this;
	}

	template <typename T, typename... Ts> requires requires(T t, Ts... ts) {
		std::wcout << t;
		(std::wcout << ... << std::forward<Ts>(ts));
	}
	PublicLogger& print(T&& msg, Ts&&... other) noexcept {
		std::wstringstream stream = {};
		stream << L"          " << name << L"        " << msg;
		prints(stream, other...);
		stream << std::endl;
		const String& str = stream.str();
		mainLogger << str;
		atomicAcquire();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE);
		std::wcout << str;
		atomicRelease();
		return *this;
	}

	template <typename T, typename... Ts> requires requires(std::wstringstream stream, T&& t, Ts&&... ts) {
		stream << std::forward<T>(t);
		(stream << ... << std::forward<Ts>(ts));
	}
	LoggableString of(T&& t, Ts&&... ts) {
		std::wstringstream stream = {};
		stream << std::forward<T>(t);
		ofs(stream, ts...);
		return LoggableString(stream.str(), this);
	}

	template <typename T> requires requires(std::wstringstream stream, T&& t) { std::wstringstream(t); }
	LoggableString of(T&& ts) {
		std::wstringstream stream = {};
		ofs(stream, ts);
		return LoggableString(stream.str(), this);
	}
};
