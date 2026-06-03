
#pragma once

#include <array>
#include <atomic>

class InteractManager {
	const wchar_t* names[256] {};
	std::array<std::atomic<bool>, 256> status{};
	std::array<std::atomic<bool>, 256> tested{};
public:
	void keydown(int keyCode) noexcept;
	void keyup(int keyCode) noexcept;
	void update(int keyCode, bool isDown) noexcept;
	bool consume(int keyCode) noexcept;
	bool peek(int keyCode) const noexcept;
	void enableRawInput() noexcept;
};
