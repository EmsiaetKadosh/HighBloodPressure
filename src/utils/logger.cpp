
#include <Windows.h>
#include <iostream>
#include <deque>
#include "src\utils\logger.hpp"
#include "src\render\string.hpp"
#include "src\utils\atomic.hpp"
#include "src\main.hpp"

struct LogLine {
	RenderableString head;
	RenderableString body;

	[[nodiscard]] String getString() const noexcept { return head.getString() + body.getString(); }
};

class LoggerImpl final : public PublicLogger {
	Deque<LogLine> deque {};
	mutable AtomicLock lock {};
	unsigned int saveCount = 0;

	static String prepareHead(String&& type) { return L"\\r\\#ffeeeeee" + (game.isReady() ? qwtowb10(game.getTick(), 8) : L"        ") + type; }

	void outputString(String&& head, RenderableString&& content) {
		lock.acquire();
		deque.emplace_back(std::move(head), std::move(content));
		std::wcout << deque.back().getString() << std::endl;
		lock.release();
	}

public:
	LoggerImpl() noexcept = default;

	PublicLogger& error(RenderableString&& content) noexcept override { return outputString(prepareHead(L" \\#ffee0000ERROR \\#ffeeeeee"), std::move(content)), *this; }
	PublicLogger& warn(RenderableString&& content) noexcept override { return outputString(prepareHead(L" \\#ffeeee00WARN  \\#ffeeeeee"), std::move(content)), *this; }
	PublicLogger& info(RenderableString&& content) noexcept override { return outputString(prepareHead(L" \\#ffeeeeeeINFO  \\#ffeeeeee"), std::move(content)), *this; }
	PublicLogger& log(RenderableString&& content) noexcept override { return outputString(prepareHead(L" \\#ffccccccLOG   \\#ffeeeeee"), std::move(content)), *this; }
	PublicLogger& debug(RenderableString&& content) noexcept override { return outputString(prepareHead(L" \\#ff88cceeDEBUG \\#ffeeeeee"), std::move(content)), *this; }
	PublicLogger& trace(RenderableString&& content) noexcept override { return outputString(prepareHead(L" \\#ff4488eeTARCE \\#ffeeeeee"), std::move(content)), *this; }
	void setSaveCount(const unsigned int count) override { saveCount = count; }
};

PublicLogger& Details::getPublicLogger() {
	static LoggerImpl impl;
	return impl;
}

LoggableString::LoggableString(RenderableString&& renderableString): renderableString(new RenderableString(std::move(renderableString))) {}
LoggableString::~LoggableString() { delete renderableString; }
void LoggableString::error() const noexcept { Logger.error(RenderableString(*renderableString)); }
void LoggableString::warn() const noexcept { Logger.warn(RenderableString(*renderableString)); }
void LoggableString::info() const noexcept { Logger.info(RenderableString(*renderableString)); }
void LoggableString::log() const noexcept { Logger.log(RenderableString(*renderableString)); }
void LoggableString::debug() const noexcept { Logger.debug(RenderableString(*renderableString)); }
void LoggableString::trace() const noexcept { Logger.trace(RenderableString(*renderableString)); }
