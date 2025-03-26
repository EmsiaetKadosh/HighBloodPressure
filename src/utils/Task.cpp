//
// Created by EmsiaetKadosh on 25-3-23.
//

#include "Task.h"

#include "..\game\Game.h"

void Task::initParams() { triggerInterval = 0, nextExecuteTime = game.getTick(), lastExecuteTime = game.getTick(); }
Task& Task::every(const QWORD tick) { return triggerInterval = tick, *this; }
Task& Task::after(const QWORD tick) { return nextExecuteTime = game.getTick() + tick, *this; }
Task& Task::until(const QWORD tick) { return lastExecuteTime = game.getTick() + tick, *this; }
Task& Task::forever() { return lastExecuteTime = static_cast<QWORD>(-1), *this; }

void TaskScheduler::runAll() {
	const QWORD tick = game.getTick();
	for (Task& task : tasks) {
		if (tick == task.nextExecuteTime) {
			task.func(task);
			task.nextExecuteTime += task.triggerInterval;
		}
		if (tick >= task.lastExecuteTime || task.scheduledPop()) {
			task.schedulePop(false);
			tasks.pop(&task);
		}
	}
}

void TaskScheduler::pushCopy(Task& task) {
	tasks.pushCopy(&task);
	if (game.getTick() >= task.lastExecuteTime) task.lastExecuteTime = game.getTick();
	if (game.getTick() >= task.nextExecuteTime) task.nextExecuteTime = game.getTick();
}

void TaskScheduler::pushNewed(Task* task) {
	tasks.pushNewed(task);
	if (game.getTick() >= task->lastExecuteTime) task->lastExecuteTime = game.getTick();
	if (game.getTick() >= task->nextExecuteTime) task->nextExecuteTime = game.getTick();
}

void TaskScheduler::pushThis(Task& task) {
	tasks.pushThis(&task);
	if (game.getTick() >= task.lastExecuteTime) task.lastExecuteTime = game.getTick();
	if (game.getTick() >= task.nextExecuteTime) task.nextExecuteTime = game.getTick();
}
