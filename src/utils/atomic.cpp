
#include <Windows.h>
#include <thread>
#include "src\utils\atomic.hpp"
#include "src\main.hpp"


inline AtomicGuard::AtomicGuard(const AtomicLock* storage) : storage(storage) { if (storage) storage->acquire(); }
AtomicGuard::AtomicGuard(AtomicGuard&& other) noexcept : storage(other.storage) { other.storage = nullptr; }
inline AtomicGuard::~AtomicGuard() noexcept { if (storage && storage->isActive() && storage->isAcquired()) storage->release(); }

AtomicGuard& AtomicGuard::operator=(AtomicGuard&& other) noexcept {
	if (this == &other) return *this;
	storage = other.storage;
	other.storage = nullptr;
	return *this;
}

AtomicLock::~AtomicLock() { atomicActiveFlag = false; }

void AtomicLock::acquire(const bool isLong) const noexcept {
	bool expected;
	unsigned int spinCount = 0;
	while (expected = false, atomicActiveFlag && !atomicFlag.compare_exchange_strong(expected, true)) ++spinCount, std::this_thread::sleep_for(std::chrono::microseconds(1));
	const bool lw = longWaiting;
	longWaiting = isLong;
	if (doPrint && spinCount) Logger.of(lw ? L"long" : L"short", L"AtomicLock::acquired for", spinCount, L"times").trace();
}

void AtomicLock::release() const noexcept {
	bool expected;
	while (expected = true, atomicActiveFlag && !atomicFlag.compare_exchange_strong(expected, false)) std::this_thread::sleep_for(std::chrono::microseconds(100)), Logger.of(L"?? AtomicLock::release waiting...").warn();
}
