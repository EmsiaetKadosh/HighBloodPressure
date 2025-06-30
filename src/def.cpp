//
// Created by EmsiaetKadosh on 25-1-18.
//

#include "def.h"

#include "utils\exception.h"
#include "utils\utils.h"

void requireNonnull(const void* value) noexcept(false) { if (!value) throw NullPointerException(L"value is null"); }
void checkAllocation(const void* value) noexcept(false) { if (!value) throw BadAllocationException(L"bad allocation"); }

[[noreturn]] void unreachable() noexcept(false) {
	Logger.fatal(L"Unreachable code executed");
	printStacktrace(1);
	throw RuntimeException(L"Unreachable code executed");
}
