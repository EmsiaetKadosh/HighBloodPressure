
#pragma once

#include <atomic>

class AtomicGuard {
	friend class AtomicLock;
	const AtomicLock* storage;

public:
	AtomicGuard(const AtomicLock* storage);
	AtomicGuard(const AtomicGuard&) = delete;
	AtomicGuard(AtomicGuard&& other) noexcept;
	~AtomicGuard() noexcept;
	AtomicGuard& operator=(const AtomicGuard& other) = delete;

	AtomicGuard& operator=(AtomicGuard&& other) noexcept;
};

class AtomicLock final {
	mutable std::atomic_bool atomicFlag = false;
	mutable bool atomicActiveFlag = true;
	mutable bool longWaiting = false;
	mutable bool doPrint = false;

public:
	~AtomicLock();

	void acquire(bool isLong = false) const noexcept;
	void release() const noexcept;
	bool isActive() const noexcept { return atomicActiveFlag; }
	bool isAcquired() const noexcept { return atomicFlag; }
	AtomicGuard getGuard() const noexcept { return AtomicGuard(this); }
};
