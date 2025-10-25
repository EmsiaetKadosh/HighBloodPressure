
#pragma once

#include <sstream>
#include "src\render\string.hpp"

class PublicLogger;

namespace Details {
	extern PublicLogger& getPublicLogger();
}

class LoggableString {
	RenderableString* renderableString;
	friend class PublicLogger;
	LoggableString(RenderableString&& renderableString);

public:
	~LoggableString();
	void error() const noexcept;
	void warn() const noexcept;
	void info() const noexcept;
	void log() const noexcept;
	void debug() const noexcept;
	void trace() const noexcept;
};

class PublicLogger {
public:
	static PublicLogger& getInstance() noexcept { Details::getPublicLogger(); }
	virtual ~PublicLogger() = default;

	virtual PublicLogger& error(RenderableString&&) noexcept = 0;
	virtual PublicLogger& warn(RenderableString&&) noexcept = 0;
	virtual PublicLogger& info(RenderableString&&) noexcept = 0;
	virtual PublicLogger& log(RenderableString&&) noexcept = 0;
	virtual PublicLogger& debug(RenderableString&&) noexcept = 0;
	virtual PublicLogger& trace(RenderableString&&) noexcept = 0;
	virtual void setSaveCount(unsigned int = 100) = 0;
	[[nodiscard]] LoggableString of(RenderableString&& content) noexcept { return LoggableString(std::move(content)); }

	template <typename... Args> requires requires(std::wstringstream& str) {
		(str << ... << std::forward<Args>(std::declval<Args>()));
	}
	[[nodiscard]] LoggableString of(Args&&... args) {
		std::wstringstream stream;
		(stream << ... << std::forward<Args>(args));
		return RenderableString(stream.str());
	}
};
