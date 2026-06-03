
#pragma once

class InteractManager {
	const wchar_t* names[256] {};
	bool status[256] {};
	bool tested[256] {};
public:
	void keydown(int keyCode) noexcept; // 设置按下状态
	void keyup(int keyCode) noexcept; // 设置松开状态
	void update(int keyCode, bool isDown) noexcept; // 更新按键状态
	bool consume(int keyCode) noexcept; // 检测并消耗按下状态
	bool peek(int keyCode) const noexcept; // 忽略是否已经背检测，直接获取实时状态
	void enableRawInput() noexcept;
};
