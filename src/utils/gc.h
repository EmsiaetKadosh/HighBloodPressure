//
// Created by EmsiaetKadosh on 25-3-6.
//

#pragma once

#include "..\warnings.h"
#include "..\def.h"
#include "exception.h"

struct IGarbage;
template <typename T>
class Garbage;
class GarbageCollector;

struct IGarbage {
private:
	friend class GarbageCollector;
	IGarbage* next = nullptr;

protected:
	void* ptr;
	IGarbage(void* ptr) : ptr(ptr) {}
	virtual ~IGarbage() = default;
	virtual void collect() = 0;
	virtual void deleteThis() = 0;
};

namespace $LimitedAccess {
	/**
	 * 仅仅是为了garbage使用的
	 */
	String garbageStacktraceString(const std::stacktrace&) noexcept;
}

template <typename T>
class Garbage final : public IGarbage {
	friend class GarbageCollector;
	std::stacktrace submitContext;

public:
	Garbage(T* ptr) : IGarbage(ptr), submitContext(std::stacktrace::current()) {}
	void collect() override { delete static_cast<T*>(deallocating_message(ptr, $LimitedAccess::garbageStacktraceString(submitContext))); }

protected:
	void deleteThis() override { delete deallocating(this); }
};

class GarbageCollector {
	IGarbage* submitted = nullptr;
	IGarbage* submittedEnd = nullptr;
	IGarbage* packed = nullptr;
	IGarbage* packedEnd = nullptr;
	IGarbage* processing = nullptr;

public:
	GarbageCollector() { Logger.info(L"initialize GarbageCollector"); }
	GarbageCollector(const GarbageCollector&) = delete;
	GarbageCollector(GarbageCollector&&) = delete;
	GarbageCollector& operator=(const GarbageCollector&) = delete;
	GarbageCollector& operator=(GarbageCollector&&) = delete;

	~GarbageCollector() {
		IGarbage* iter;
		while (processing) {
			iter = processing->next;
			processing->collect();
			processing->deleteThis();
			processing = iter;
		}
		while (packed) {
			iter = packed->next;
			packed->collect();
			packed->deleteThis();
			packed = iter;
		}
		while (submitted) {
			iter = submitted->next;
			submitted->collect();
			submitted->deleteThis();
			submitted = iter;
		}
		submittedEnd = nullptr;
	}

	/** 只能在gameThread调用 */
	template<TypeName T>
	void submit(T* ptr) noexcept(false) {
		IGarbage* newedGarbage = allocatedFor(new Garbage<T>(ptr));
		builtinSubmit(newedGarbage);
	}

	void builtinSubmit(IGarbage* const newedGarbage) noexcept(false) {
		if (IGarbage* end = submittedEnd) { // 后续添加，可能存在线程竞争
			while (end->next) end = end->next; // 理论上不会进入循环，但防止万一
			end->next = newedGarbage;
			if (submitted) submittedEnd = newedGarbage; // 参考pack()，submitted会被先置空
			std::atomic_thread_fence(std::memory_order_acquire);
			if (!submitted) submittedEnd = nullptr; // 防止submittedEnd = garbage在pack()中置空后进行。此函数是同步的，所以可以这样操作。
		}
		else { // 添加首个，一定不会有线程竞争；或先前的已经pack
			submitted = newedGarbage;
			submittedEnd = newedGarbage;
		}
	}

	/** 只能在renderThread调用 */
	void pack() {
		if (!submitted) return; // 提交链为空，不进行后续操作，防止竞争
		std::atomic_thread_fence(std::memory_order_acquire);
		IGarbage* submit = submitted;
		IGarbage* submitEnd = submittedEnd;
		submitted = nullptr; // 先把这个置为nullptr
		std::atomic_thread_fence(std::memory_order_acquire);
		submittedEnd = nullptr;
		if (IGarbage* end = packedEnd) {
			while (end->next) end = end->next; // 以防万一
			end->next = submit;
			if (packed) packedEnd = submitEnd;
			std::atomic_thread_fence(std::memory_order_acquire);
			if (!packed) packedEnd = nullptr;
		}
		else {
			packed = submit;
			packedEnd = submitEnd;
		}
		while (packedEnd->next) packedEnd = packedEnd->next;
	}

	/** 只能在gameThread调用 */
	void collect() {
		IGarbage* next;
		while (processing) {
			next = processing->next;
			processing->next = nullptr;
			processing->collect();
			processing->deleteThis();
			processing = next;
		}
		if (!packed) return;
		// 搬运packed且搬运期间的packed链必须必须完全固定
		// 保证运行顺序
		processing = packed;
		packed = nullptr;
		std::atomic_thread_fence(std::memory_order_acquire);
		packedEnd = nullptr;
		std::atomic_thread_fence(std::memory_order_acquire);
		while (processing) {
			next = processing->next;
			processing->next = nullptr;
			processing->collect();
			processing->deleteThis();
			processing = next;
		}
	}
};

inline GarbageCollector& [[carlbeks::releasedat("def.cpp")]] gc = *new GarbageCollector();
