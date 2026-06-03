
#pragma once

#include <stacktrace>
#include <thread>
#include <sstream>
#include <src\using.hpp>
#include <src\render\string.hpp>

class Exception {
protected:
	String description;
	Stacktrace stacktrace;

	Exception(String&& desc);
	virtual ~Exception() = default;

public:
	[[nodiscard]] const String& getDescription() const noexcept { return description; }
	[[nodiscard]] const Stacktrace& getStacktrace() const noexcept { return stacktrace; }
	[[nodiscard]] String getStacktraceString() const noexcept;
	Exception& printStacktrace() noexcept;
	Exception& crash(const String& prefix) noexcept;

	[[nodiscard]] virtual String getName() const noexcept = 0;
};

inline String stacktraceString(const std::stacktrace& stacktrace, unsigned int maxCount = 5) noexcept;
inline RenderableString stacktraceStringRenderable(const std::stacktrace& stacktrace, unsigned int maxCount = 5) noexcept;

class ThreadInterferenceException final : public Exception {
	Thread::id id;

public:
	ThreadInterferenceException(String&& desc) : Exception(std::move(desc)), id(std::this_thread::get_id()) { description += (std::wostringstream() << L" (Current Thread: " << id << L')').str(); }
	~ThreadInterferenceException() override = default;
	[[nodiscard]] String getName() const noexcept override { return L"ThreadInterferenceExeception"; }
};

class NotImplementedException final : public Exception {
public:
	NotImplementedException(const String& v_func_) : Exception(v_func_ + L" is not implemented yet.") {}
	~NotImplementedException() override = default;
	[[nodiscard]] String getName() const noexcept override { return L"NotImplementedException"; }
};
