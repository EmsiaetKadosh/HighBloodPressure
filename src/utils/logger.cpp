
#include <string_view>
#include <iostream>
#include <deque>
#include "src\main.hpp"


#if false
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
#endif

LoggerRouter& Details::getLoggerRouter() {
	static LoggerRouter impl;
	return impl;
}

LoggableString::LoggableString(DomainLogger& logger, RenderableString&& renderableString) noexcept : logger(logger), renderableString(new RenderableString(std::move(renderableString))) {}
LoggableString::~LoggableString() { delete renderableString; }
void LoggableString::error() const noexcept { logger.error(RenderableString(*renderableString)); }
void LoggableString::warn() const noexcept { logger.warn(RenderableString(*renderableString)); }
void LoggableString::info() const noexcept { logger.info(RenderableString(*renderableString)); }
void LoggableString::log() const noexcept { logger.log(RenderableString(*renderableString)); }
void LoggableString::debug() const noexcept { logger.debug(RenderableString(*renderableString)); }
void LoggableString::trace() const noexcept { logger.trace(RenderableString(*renderableString)); }

RenderableString DomainLogger::prepareHead(String&& type) const noexcept {
	try {
		String head = String();
		constexpr StringView h = L"\\r\\#ffeeeeee";
		head.reserve(h.length() + 9 + type.length() + 4);
		head.append(h);
		if (game.isReady()) head.append(qwtowb10(game.getTick(), 8));
		else head.append(' ', 8);
		head.append(' ', 1);
		head.append(type);
		head.append(' ', 1);
		RenderableString ret = std::move(head);
		ret.append(domain);
	}
	catch (...) { helperLoggerDirect(L"ERROR-HEAD"); }
	return RenderableString();
}

void DomainLogger::helperLoggerDirect(const wchar* content) const noexcept { logger.outputDirect(content); }

DomainLogger& DomainLogger::error(RenderableString&& content) noexcept {
	logger.outputString(prepareHead(L"\\#ffee0000ERROR \\#ffeeeeee"), std::move(content));
	return *this;
}

DomainLogger& DomainLogger::warn(RenderableString&& content) noexcept {
	logger.outputString(prepareHead(L"\\#ffeeee00WARN  \\#ffeeeeee"), std::move(content));
	return *this;
}

DomainLogger& DomainLogger::info(RenderableString&& content) noexcept {
	logger.outputString(prepareHead(L"\\#ffeeeeeeINFO  \\#ffeeeeee"), std::move(content));
	return *this;
}

DomainLogger& DomainLogger::log(RenderableString&& content) noexcept {
	logger.outputString(prepareHead(L"\\#ffccccccLOG   \\#ffeeeeee"), std::move(content));
	return *this;
}

DomainLogger& DomainLogger::debug(RenderableString&& content) noexcept {
	logger.outputString(prepareHead(L"\\#ff88cceeDEBUG \\#ffeeeeee"), std::move(content));
	return *this;
}

DomainLogger& DomainLogger::trace(RenderableString&& content) noexcept {
	logger.outputString(prepareHead(L"\\#ff4488eeTARCE \\#ffeeeeee"), std::move(content));
	return *this;
}

void LoggerRouter::outputString(RenderableString&& head, RenderableString&& content) noexcept {
	lock.acquire();
	try {
		deque.emplace_back(std::move(head), std::move(content));
		std::wcout << deque.back().getString() << std::endl;
	}
	catch (std::bad_alloc&) {
		std::wcout << L"Cannot generate RenderableString: BAD_ALLOC\n @ ";
		std::cout << __FUNCSIG__ << std::endl;
	}
	catch (std::exception& e) {
		std::wcout << L"Cannot generate RenderableString: ";
		std::cout << e.what() << "\n @ " << __FUNCSIG__ << std::endl;
	}
	lock.release();
}

void LoggerRouter::outputDirect(const wchar* const content) noexcept {
	lock.acquire();
	try {
		std::wcout << content << std::endl;
		try { deque.emplace_back(String(), content); }
		catch (std::bad_alloc&) {
			game.getRiskManager().report();
			std::wcout << L"Cannot generate RenderableString: BAD_ALLOC\n @ ";
			std::cout << __FUNCSIG__ << std::endl;
		}
		catch (std::exception& e) {
			std::wcout << L"Cannot generate RenderableString: ";
			std::cout << e.what() << "\n @ " << __FUNCSIG__ << std::endl;
		}
	}
	catch (...) {}
	lock.release();
}

void LoggerRouter::setSaveCount(const unsigned int count) noexcept { saveCount = count; }
DomainLogger LoggerRouter::ofDomain(RenderableString&& domain) noexcept { return DomainLogger(*this, std::move(domain)); }
