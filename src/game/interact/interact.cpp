
#include "interact.hpp"

void InteractManager::keydown(const int keyCode) noexcept { if (!status[keyCode]) status[keyCode] = true, tested[keyCode] = false; }
void InteractManager::keyup(const int keyCode) noexcept { tested[keyCode] = true, status[keyCode] = false; }

void InteractManager::update(const int keyCode, const bool isDown) noexcept {
	if (isDown) keydown(keyCode);
	else keyup(keyCode);
}

bool InteractManager::consume(const int keyCode) noexcept {
	if (tested[keyCode]) return false;
	return tested[keyCode] = true, status[keyCode];
}

bool InteractManager::peek(const int keyCode) const noexcept { return status[keyCode]; }
void InteractManager::enableRawInput() noexcept {}
