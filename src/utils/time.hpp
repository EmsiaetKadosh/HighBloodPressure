
#pragma once

#include <chrono>
#include <thread>

#include "src\using.hpp"

inline long long nanoCurrent() noexcept { return SystemClock::now().time_since_epoch().count(); }

class TimeManager {
	TimePoint startTime = Clock::now();

public:
	static TimeManager& getInstance() noexcept {
		static TimeManager time {};
		return time;
	}

	[[nodiscard]] long long getCurrentTime() const noexcept { return (Clock::now() - startTime).count(); }
	[[nodiscard]] double getCurrentMilliseconds() const noexcept { return static_cast<double>(getCurrentTime()) / 1'000.0; }
	[[nodiscard]] double getCurrentSeconds() const noexcept { return static_cast<double>(getCurrentTime()) / 1'000'000.0; }
};

namespace Details {
	inline TimeManager& getTimeManager() noexcept { return TimeManager::getInstance(); }
}

class Timer {
	TimePoint startTime = Clock::now();

public:
	[[nodiscard]] long long elapsed() const noexcept { return (Clock::now() - startTime).count(); }
	[[nodiscard]] double elapsedMilliseconds() const noexcept { return static_cast<double>(elapsed()) / 1'000.0; }
	[[nodiscard]] double elapsedSeconds() const noexcept { return static_cast<double>(elapsed()) / 1'000'000.0; }
	void reset() noexcept { startTime = Clock::now(); }

	void waitUntilAndReset(long long ns) noexcept {
		long long th = (Clock::now() - startTime).count();
		if (th >= ns) return;
		th = ns - th; // remain
		constexpr long long _20us = 20'000;
		if (th > _20us) std::this_thread::sleep_for(std::chrono::nanoseconds(th - _20us));
		while ((Clock::now() - startTime).count() - ns < 0) _mm_pause();
	}

	long long lap() noexcept {
		const TimePoint now = Clock::now();
		const long long duration = (now - startTime).count();
		startTime = now;
		return duration;
	}

	double lapMilliseconds() noexcept {
		const TimePoint now = Clock::now();
		const long long duration = (now - startTime).count();
		startTime = now;
		return static_cast<double>(duration) / 1'000.0;
	}

	double lapSeconds() noexcept {
		const TimePoint now = Clock::now();
		const long long duration = (now - startTime).count();
		startTime = now;
		return static_cast<double>(duration) / 1'000'000.0;
	}
};

