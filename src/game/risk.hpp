//
// Created by EmsiaetKadosh on 2026/2/8.
//

#pragma once

enum class RiskCategory {
	MEMORY,
	PERFORMANCE,
	SYSTEM,
	LOGIC,
	UNKNOWN
};

enum class MemoryRiskType {
	CRITICAL_LOW_MEMORY, // 内存即将耗尽
	HIGH_FRAGMENTATION, // 内存碎片严重
	LEAK_SUSPECTED, // 疑似内存泄漏
	VRAM_OVERFLOW, // 显存溢出
	BUFFER_OVERFLOW, // 缓冲区溢出风险
	STACK_OVERFLOW_RISK, // 栈溢出风险
	MEMORY_ALLOC_FAILURE // 内存分配失败
};

enum class PerformanceRiskType {
	FRAME_TIME_SPIKE, // 帧时间突增
	CONSISTENT_LOW_FPS, // 持续低帧率
	MAIN_THREAD_BLOCKED, // 主线程阻塞
	RENDER_THREAD_STARVED, // 渲染线程饥饿
	PHYSICS_OVERLOAD, // 物理系统过载
	GPU_BOTTLENECK, // GPU瓶颈
	INPUT_LAG_DETECTED // 输入延迟检测
};

enum class SystemRiskType {
	DISK_SPACE_LOW, // 磁盘空间不足
	DISK_FRAGMENTED, // 磁盘碎片严重
	TEMPERATURE_HIGH, // 温度过高
	POWER_LOW, // 电量不足（移动设备）
	NETWORK_UNSTABLE, // 网络不稳定
	OS_RESOURCE_LOW, // 系统资源不足
	ANTIVIRUS_INTERFERENCE // 杀毒软件干扰
};

enum class LogicRiskType {
	DEADLOCK_POTENTIAL, // 潜在死锁
	RACE_CONDITION_RISK, // 竞态条件风险
	INFINITE_LOOP_SUSPECTED, // 疑似无限循环
	CORRUPTED_SAVE_DATA, // 存档数据损坏
	INVALID_STATE_TRANSITION, // 无效状态转移
	DEPENDENCY_CIRCULAR, // 循环依赖风险
	GAME_LOGIC_TIMEOUT // 游戏逻辑超时
};

enum class OperationMode {
	ULTRA_PERFORMANCE, // 极限性能模式（牺牲质量）
	BALANCED, // 平衡模式（默认）
	CONSERVATIVE, // 保守模式（注重稳定性）
	DEGRADED, // 降级模式（功能受限）
	MAINTENANCE // 维护模式（仅核心功能）
};

class RiskReport {
public:
	RiskReport() noexcept;
};

class GameCrashRiskManager {
public:
	void report() noexcept;
	void check() noexcept;
};
