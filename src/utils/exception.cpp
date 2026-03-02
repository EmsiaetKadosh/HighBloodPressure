
#include <sstream>
#include "src\utils\exception.hpp"
#include "chars.hpp"
#include "src\main.hpp"

Exception::Exception(String&& desc) : description(std::move(desc)), stacktrace(Stacktrace::current()) {}

String Exception::getStacktraceString() const noexcept {
	for (const std::stacktrace_entry& entry : stacktrace) {}
	return L"Not implemented"; // TODO(EmsiaetKadosh): stacktrace
}

Exception& Exception::printStacktrace() noexcept {
	Logger.error(stacktraceStringRenderable(stacktrace, 0));
	return *this;
}

Exception& Exception::crash(const String& prefix) noexcept {
	game.crash(prefix.empty() ? description : prefix + L": " + description);
	return *this;
}

String stacktraceString(const std::stacktrace& stacktrace, unsigned int maxCount) noexcept {
	if (!maxCount) return L"Stacktrace: 0";
	std::wstringstream ss;
	ss << L"Stacktrace:";
	for (const auto& entry : stacktrace) {
		ss << L"\n        Calling " << u8to16w_nothrow(entry.description());
		if (const std::string str = entry.source_file(); !str.empty()) ss << L" @ " << u8to16w_nothrow(entry.source_file()) << L":" << entry.source_line();
		if (!--maxCount) break;
	}
	return ss.str();
}

RenderableString stacktraceStringRenderable(const std::stacktrace& stacktrace, unsigned int maxCount) noexcept {
	if (!maxCount) return L"\\#ffee0000Stacktrace: 0";
	std::wstringstream ss;
	ss << L"\\#ffee0000Stacktrace:";
	for (const auto& entry : stacktrace) {
		ss << L"\n\\#ffeeeeee        Calling \\#ffdcdcaa" << u8to16w_nothrow(entry.description());
		if (const std::string str = entry.source_file(); !str.empty()) ss << L" \\#ffaaaaaa@\\#88bbee " << u8to16w_nothrow(entry.source_file()) << L"\\#ffaaaaaa:\\#ffb4cda8" << entry.source_line();
		if (!--maxCount) break;
	}
	return ss.str();
}
