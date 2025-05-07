//
// Created by EmsiaetKadosh on 25-3-4.
//
#pragma once

#include "utils.h"

class Task;
class TaskScheduler;

class Task final : public AnywhereEditable<Task> {
	friend class TaskScheduler;
	friend class AnywhereEditableList<Task>;
	friend class Garbage<Task>;
	QWORD triggerInterval = 0, nextExecuteTime = 0, lastExecuteTime = 0;

	void initParams();

	void* operator new(std::size_t size) { return ::operator new(size); }
	void* operator new(std::size_t size, void* ptr) noexcept { return ::operator new(size, ptr); }
	void operator delete(void* ptr) noexcept { ::operator delete(ptr); }
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

	static Task* of(const Function<void(Task&)>& func) noexcept { return allocatedFor(new Task(func)); }
	static Task* of(Function<void(Task&)>&& func) noexcept { return allocatedFor(new Task(std::move(func))); }
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

class ScopeGuard {
public:
	Functional tasks;

	ScopeGuard(const Function<void()>& task) : tasks(task) {}
	ScopeGuard(Function<void()>&& task) : tasks(task) {}
	ScopeGuard(const Functional& functional) : tasks(functional) {}
	ScopeGuard(Functional&& functional) : tasks(std::move(functional)) {}
	ScopeGuard(nullptr_t) noexcept : tasks(nullptr) {}
	ScopeGuard(const ScopeGuard& other) = delete;
	ScopeGuard(ScopeGuard&& other) = delete;
	ScopeGuard& operator=(const ScopeGuard& other) = delete;
	ScopeGuard& operator=(ScopeGuard&& other) = delete;
	~ScopeGuard() { tasks(); }

	ScopeGuard& then(const Function<void()>& next) { return tasks.then(next), *this; }
	ScopeGuard& then(Function<void()>&& next) { return tasks.then(std::move(next)), *this; }
};
