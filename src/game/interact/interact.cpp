
#include "interact.hpp"

void InteractManager::keydown(const int keyCode) noexcept {
	if (status[keyCode].load(std::memory_order_relaxed)) return;
	status[keyCode].store(true, std::memory_order_release);
	tested[keyCode].store(false, std::memory_order_release);
}
void InteractManager::keyup(const int keyCode) noexcept {
	tested[keyCode].store(true, std::memory_order_release);
	status[keyCode].store(false, std::memory_order_release);
}

void InteractManager::update(const int keyCode, const bool isDown) noexcept {
	if (isDown) {
		if (status[keyCode].load(std::memory_order_relaxed)) return;
		status[keyCode].store(true, std::memory_order_release);
		tested[keyCode].store(false, std::memory_order_release);
	} else {
		tested[keyCode].store(true, std::memory_order_release);
		status[keyCode].store(false, std::memory_order_release);
	}
}

bool InteractManager::consume(const int keyCode) noexcept {
	if (tested[keyCode].exchange(true, std::memory_order_acq_rel)) return false; // 已被消耗
	return status[keyCode].load(std::memory_order_acquire);
}

bool InteractManager::peek(const int keyCode) const noexcept {
	return status[keyCode].load(std::memory_order_relaxed);
}
void InteractManager::enableRawInput() noexcept {}
