//
// Created by EmsiaetKadosh on 25-1-18.
//

#include "def.h"

#include "utils\exception.h"
#include "utils\Chars.h"
#include "utils\gc.h"
#include "utils\utils.h"

void requireNonnull(const void* value) noexcept(false) { if (!value) throw NullPointerException(L"value is null"); }
void checkAllocation(const void* value) noexcept(false) { if (!value) throw BadAllocationException(L"bad allocation"); }

#if defined __CARLBEKS_DEBUG__ || defined __CARLBEKS_MEMORY__
#if __CARLBEKS_MEMORY__ > 2
void $LimitedAccess::printAllocate(void* value, const size_t size, const String& msg) {
	const String str = L"alloc   " + ptrtow(reinterpret_cast<QWORD>(value)) + L" " + std::to_wstring(size) + String(L"B ") + msg;
	Logger.log(str);
}
#endif

#if __CARLBEKS_MEMORY__ > 3
void $LimitedAccess::printDeallocate(void* value, const size_t size, const String& msg) {
	const String str = L"dealloc " + ptrtow(reinterpret_cast<QWORD>(value)) + L" " + std::to_wstring(size) + String(L"B ") + msg;
	Logger.log(str);
}
#endif

void $LimitedAccess::printDeallocateWarning(void* value, const String& msg) {
	const String str = L"dealloc " + ptrtow(reinterpret_cast<QWORD>(value)) + L": " + msg;
	Logger.error(str);
}

[[noreturn]] void unreachable() noexcept(false) {
	Logger.fatal(L"Unreachable code executed");
	printStacktrace(1);
	throw RuntimeException(L"Unreachable code executed");
}
#endif

String ptrtow(const QWORD value) { return qwtowb16(value, 16); }

namespace $LimitedAccess {
	Release::~Release() {
		delete &gc;
		Logger.put(L"--------- Last Check ---------");
		for (const auto& [addr, info] : memoryManager.allocated) Logger.print(L"  using", addr, info.size, L"B", info.msg);
		std::atomic_thread_fence(std::memory_order_acquire);
		delete &Logger;
		delete &memoryManager;
		std::wcout << L"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
	}

	extern Release gcRelease_LoggerRelease_memoryManagerRelease = Release();
}
