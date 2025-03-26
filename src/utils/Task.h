//
// Created by EmsiaetKadosh on 25-3-4.
//
#pragma once

#include "utils.h"

class Task;
class TaskScheduler;

class Task final : public AnywhereEditable<Task> {
	friend class TaskScheduler;
	QWORD triggerInterval = 0, nextExecuteTime = 0, lastExecuteTime = 0;

	void initParams();

public:
	Function<void(Task& self)> func;

	Task(const Function<void(Task& self)>& func) : func(func) { initParams(); }
	Task(Function<void(Task& self)>&& func) : func(std::move(func)) { initParams(); }
	Task(const Task& other) noexcept : triggerInterval(other.triggerInterval), nextExecuteTime(other.nextExecuteTime), lastExecuteTime(other.lastExecuteTime), func(other.func) {}
	Task(Task&& other) noexcept : triggerInterval(other.triggerInterval), nextExecuteTime(other.nextExecuteTime), lastExecuteTime(other.lastExecuteTime), func(std::move(other.func)) {}
	void schedulePop(const bool pop) noexcept { reserved[0] = pop; }
	[[nodiscard]] bool scheduledPop() const noexcept { return reserved[0]; }

	int pop() noexcept override {
		schedulePop(true);
		Success();
	}

	Task& every(QWORD tick);
	Task& after(QWORD tick);
	Task& until(QWORD tick);
	Task& forever();
};

class TaskScheduler {
public:
	AnywhereEditableList<Task> tasks;

	void runAll();

	void pushCopy(Task& task);
	/* 必须是new Task，交付托管 */
	void pushNewed(Task* task);
	void pushThis(Task& task);
};
