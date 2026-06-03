
#pragma once

#include <deque>
#include <sstream>
#include "src\utils\atomic.hpp"
#include "src\render\string.hpp"
#include "src\using.hpp"

class LoggerRouter;
class DomainLogger;

namespace Details {
	extern LoggerRouter& getLoggerRouter();
}

class LoggableString {
	DomainLogger& logger;
	RenderableString* renderableString;
	friend class PublicLogger;
	friend class DomainLogger;
	LoggableString(DomainLogger& logger, RenderableString&& renderableString) noexcept;

public:
	~LoggableString();
	void error() const noexcept;
	void warn() const noexcept;
	void info() const noexcept;
	void log() const noexcept;
	void debug() const noexcept;
	void trace() const noexcept;
};

struct LogLine {
	RenderableString head;
	RenderableString body;
	LogLine(RenderableString&& head, RenderableString&& body) noexcept : head(std::move(head)), body(std::move(body)) {}
	[[nodiscard]] String getString() const { return head.getString() + body.getString(); }
	LogLine(LogLine&& other) noexcept = default;
};

class DomainLogger {
	friend class LoggerRouter;
	RenderableString domain;
	LoggerRouter& logger;

	RenderableString prepareHead(String&& type) const noexcept;
	void helperLoggerDirect(const wchar* content) const noexcept; // 给template的ofNoexcept提供的辅助函数

	DomainLogger(LoggerRouter& src, RenderableString&& domain) : domain(std::move(domain)), logger(src) {}

public:
	DomainLogger(const DomainLogger&) = delete;
	DomainLogger& operator=(const DomainLogger&) = delete;

	DomainLogger& error(RenderableString&& content) noexcept;
	DomainLogger& warn(RenderableString&& content) noexcept;
	DomainLogger& info(RenderableString&& content) noexcept;
	DomainLogger& log(RenderableString&& content) noexcept;
	DomainLogger& debug(RenderableString&& content) noexcept;
	DomainLogger& trace(RenderableString&& content) noexcept;

	[[nodiscard]] LoggableString of(RenderableString&& content) noexcept { return LoggableString(*this, std::move(content)); }

	template <typename... Args> requires requires(std::wstringstream& str) {
		(str << ... << std::forward<Args>(std::declval<Args>()));
	}
	[[nodiscard]] LoggableString of(Args&&... args) {
		std::wstringstream stream;
		(stream << ... << std::forward<Args>(args));
		return LoggableString(*this, RenderableString(stream.str()));
	}

	template <typename... Args> requires requires(std::wstringstream& str) {
		(str << ... << std::forward<Args>(std::declval<Args>()));
	}
	[[nodiscard]] LoggableString ofNoexcept(Args&&... args) noexcept {
		try { return of(std::forward<Args&&>(args)...); }
		catch (std::bad_alloc& exception) {
			helperLoggerDirect(L"DomainLogger::ofNoexcept failure");
			return of(RenderableString());
		}
		catch (std::exception& exception) { return of(RenderableString(String(L"DomainLogger::ofNoexcept failure"), nullptr)); }
	}
};

class LoggerRouter {
	friend LoggerRouter& Details::getLoggerRouter();
	Deque<LogLine> deque = Deque<LogLine>();
	mutable AtomicLock lock = {};
	unsigned int saveCount = 0;

	LoggerRouter() noexcept = default;

public:
	static LoggerRouter& getInstance() noexcept { return Details::getLoggerRouter(); }
	LoggerRouter(const LoggerRouter&) = delete;
	LoggerRouter& operator=(const LoggerRouter&) = delete;

	void outputString(RenderableString&& head, RenderableString&& content) noexcept;
	void outputDirect(const wchar*) noexcept;
	void setSaveCount(unsigned int count = 100) noexcept;
	DomainLogger ofDomain(RenderableString&& domain) noexcept;
};
