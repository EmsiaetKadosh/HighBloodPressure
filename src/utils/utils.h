//
// Created by EmsiaetKadosh on 25-3-4.
//

#pragma once

#include "gc.h"

template <typename T, typename L = void>
class AnywhereEditable;
template <typename T, typename L = void>
class AnywhereEditableList;
template <typename T, typename L = void>
class AnywhereIterator;
class AnywhereIteratorEnd;


/**
 * 此处无法进行代码编译层面的直接约束
 * @tparam T 满足T extends AnywhereEditable<T, L>
 * @tparam L 满足L extends AnywhereEditableList<T, L>
 */
template <typename T, typename L>
class AnywhereEditable {
public:
	using Lst = std::conditional_t<std::is_same_v<L, void>, AnywhereEditableList<T, L>, L>;

private:
	friend class AnywhereIterator<T, L>;
	friend class AnywhereEditableList<T, L>;
	friend class AnywhereIteratorEnd;
	friend L;
	AnywhereEditable* prev = nullptr;
	AnywhereEditable* next = nullptr;
	AnywhereEditableList<T, L>* list = nullptr; // 指示自身属于某个列表
	bool managedByList = false; // 指示是否在列表内管理内存，而非列表外管理内存
public:
	byte reserved[7]{}; // reserved[0]: Task::schedulePop, Window::syncClose

	AnywhereEditable() = default;
	AnywhereEditable(const AnywhereEditable&) {}

	AnywhereEditable(AnywhereEditable&& other) noexcept : prev(other->prev), next(other->next), list(other->list) {
		other->prev = nullptr;
		other->next = nullptr;
		other->list = nullptr;
	}

	virtual ~AnywhereEditable() {
		if (list) {
			managedByList = false;
			auto lst = list;
			list = nullptr;
			lst->pop(static_cast<T*>(this));
			Logger.warn(L"~AnywhereEditable() : 析构时没有清除list，导致了析构时pop");
		}
	}

	int pushCopy(AnywhereEditableList<T, L>& list) noexcept;
	int pushThis(AnywhereEditableList<T, L>& list) noexcept;
	virtual int pop() noexcept;
	AnywhereEditableList<T, L>* getContainer() const noexcept { return list; }
};

template <typename T, typename L>
class AnywhereIterator {
	friend class AnywhereIteratorEnd;
	AnywhereEditable<T, L>* current;

public:
	AnywhereIterator(AnywhereEditable<T, L>* current) : current(current) {}
	AnywhereIterator(const AnywhereIterator& other) = default;

	T& operator*() noexcept(false) {
		if (current->next) return static_cast<T&>(*current);
		throw RuntimeException(L"AnywhereIterator::operator*() : next is null, end of list");
	}

	T* operator->() noexcept(false) {
		if (current->next) return &static_cast<T&>(*current);
		throw RuntimeException(L"AnywhereIterator::operator*() : next is null, end of list");
	}

	bool operator!=(const AnywhereIterator& other) const noexcept { return current != other.current; }
	bool operator==(const AnywhereIterator& other) const noexcept { return current == other.current; }
	bool operator!=(const AnywhereIteratorEnd& other) const noexcept;
	bool operator==(const AnywhereIteratorEnd& other) const noexcept;

	AnywhereIterator& operator++() noexcept(false) {
		if (current->next) current = current->next;
		else throw InvalidOperationException(L"AnywhereIterator::operator++() : end of list");
		return *this;
	}

	AnywhereIterator operator++(int) noexcept(false) {
		if (current->next) {
			AnywhereIterator ret = *this;
			current = current->next;
			return ret;
		}
		throw InvalidOperationException(L"AnywhereIterator::operator++() : end of list");
	}

	AnywhereIterator& operator--() noexcept(false) {
		if (current->prev) current = current->prev;
		else throw InvalidOperationException(L"AnywhereIterator::operator--() : begin of list");
		return *this;
	}

	AnywhereIterator operator--(int) noexcept(false) {
		if (current->prev) {
			AnywhereIterator ret = *this;
			current = current->prev;
			return ret;
		}
		throw InvalidOperationException(L"AnywhereIterator::operator--() : begin of list");
	}
};

class AnywhereIteratorEnd {
public:
	AnywhereIteratorEnd() = default;
	AnywhereIteratorEnd(const AnywhereIteratorEnd& other) = delete;
	AnywhereIteratorEnd(AnywhereIteratorEnd&& other) = delete;

	template <typename T, typename L>
	bool operator!=(const AnywhereIterator<T, L>& other) const noexcept { return other.current && other.current->next; }

	template <typename T, typename L>
	bool operator==(const AnywhereIterator<T, L>& other) const noexcept { return !operator!=(other); }
};

template <typename T, typename L>
class AnywhereEditableList {
protected:
	AnywhereEditable<T, L> head;
	AnywhereEditable<T, L> tail;

public:
	AnywhereEditableList() {
		head.next = &tail;
		tail.prev = &head;
	}

	virtual ~AnywhereEditableList() { for (AnywhereIterator<T, L> it = begin(); it != end(); ++it) {} }

	int pushCopy(T* value) noexcept;
	int pushThis(T* value) noexcept;
	int pushNewed(T* value) noexcept;
	virtual int pop(T* value) noexcept;
	AnywhereIterator<T, L> begin() noexcept { return AnywhereIterator<T, L>(head.next); }
	[[nodiscard]] AnywhereIteratorEnd end() noexcept { return {}; }
	AnywhereIterator<T, L> begin() const noexcept { return AnywhereIterator<T, L>(head.next); }
	[[nodiscard]] AnywhereIteratorEnd end() const noexcept { return {}; }
	AnywhereEditable<T, L>* front() const noexcept { return head.next == &tail ? nullptr : head.next; }
	AnywhereEditable<T, L>* back() const noexcept { return tail.prev == &head ? nullptr : tail.prev; }
};

template <typename T, typename L>
int AnywhereEditable<T, L>::pushCopy(AnywhereEditableList<T, L>& list) noexcept { return list.pushCopy(static_cast<T*>(this)); }

template <typename T, typename L>
int AnywhereEditable<T, L>::pushThis(AnywhereEditableList<T, L>& list) noexcept { return list.pushThis(static_cast<T*>(this)); }

template <typename T, typename L>
int AnywhereEditable<T, L>::pop() noexcept {
	if (!list) {
		Logger.error(L"AnywhereEditable::pop() : list is null");
		Failed();
	}
	return list->pop(static_cast<T*>(this));
}

template <typename T, typename L>
bool AnywhereIterator<T, L>::operator!=(const AnywhereIteratorEnd& other) const noexcept { return other != *this; }

template <typename T, typename L>
bool AnywhereIterator<T, L>::operator==(const AnywhereIteratorEnd& other) const noexcept { return other == *this; }

template <typename T, typename L>
int AnywhereEditableList<T, L>::pushCopy(T* value) noexcept {
	T* nv = allocatedFor(new T(*value));
	return pushNewed(nv);
}

template <typename T, typename L>
int AnywhereEditableList<T, L>::pushThis(T* value) noexcept {
	if (value->list) {
		Logger.error(L"AnywhereEditableList::pushThis() : value is already in a list");
		Failed();
	}
	value->list = this;
	tail.prev->next = value;
	value->prev = tail.prev;
	value->next = &tail;
	tail.prev = value;
	Success();
}

template <typename T, typename L>
int AnywhereEditableList<T, L>::pushNewed(T* value) noexcept {
	const int ret = pushThis(value);
	if (ret) delete deallocating(value);
	else value->managedByList = true;
	return ret;
}

template <typename T, typename L>
int AnywhereEditableList<T, L>::pop(T* value) noexcept {
	if (value->list != this) {
		Logger.error(L"AnywhereEditableList::pop() : value is not in this list");
		Failed();
	}
	value->list = nullptr;
	value->next->prev = value->prev;
	value->prev->next = value->next;
	if (value->managedByList) gc.submit(value);
	Success();
}

//
// Time
//
namespace $LimitedAccess {
	inline const Time startTime = std::chrono::system_clock::now();
}

inline Time getCurrentTime() noexcept {
	using namespace std::chrono;
	return system_clock::now();
}

inline NanoDuration getRunTime() noexcept {
	using namespace std::chrono;
	return duration_cast<nanoseconds>(system_clock::now() - $LimitedAccess::startTime);
}

//
// StackTrace
//
inline void printStackTrace(const std::stacktrace& stacktrace, int maxCount = 5, const int type = 0) noexcept {
	if (!maxCount) return;
	std::wstringstream ss;
	ss << L"Stacktrace:";
	const bool flag = maxCount < 0;
	if (flag) maxCount = -maxCount;
	if (type)
		for (const auto& entry : stacktrace) {
			ss << L"\n        Calling " << atow(entry.description().c_str()) << L" #" << entry.native_handle();
			if (const std::string str = entry.source_file(); !str.empty()) ss << L"\n            @ " << atow(entry.source_file().c_str()) << L":" << entry.source_line();
			if (!--maxCount) break;
		}
	else
		for (const auto& entry : stacktrace) {
			ss << L"\n        Calling " << atow(entry.description().c_str());
			if (const std::string str = entry.source_file(); !str.empty()) ss << L" @ " << atow(entry.source_file().c_str()) << L":" << entry.source_line();
			if (!--maxCount) break;
		}
	if (flag) Logger.error(ss.str());
	else Logger.trace(ss.str());
}

inline void printStackTrace(const unsigned int skip = 0, const int maxCount = 5, const int type = 0) {
	if (!maxCount) return;
	const std::stacktrace stack = std::stacktrace::current(1 + skip);
	printStackTrace(stack, maxCount, type);
}

//
// AtomicStorage | MultiThread
//
namespace $LimitedAccess {
	class AtomicStorageBase;
}

class AtomicGuard {
	friend class $LimitedAccess::AtomicStorageBase;
	const $LimitedAccess::AtomicStorageBase* storage;

public:
	AtomicGuard(const $LimitedAccess::AtomicStorageBase* storage);
	AtomicGuard(const AtomicGuard&) = delete;
	AtomicGuard(AtomicGuard&& other) noexcept : storage(other.storage) { other.storage = nullptr; }
	~AtomicGuard() noexcept;
	AtomicGuard& operator=(const AtomicGuard& other) = delete;

	AtomicGuard& operator=(AtomicGuard&& other) noexcept {
		if (this == &other) return *this;
		storage = other.storage;
		other.storage = nullptr;
		return *this;
	}
};

class $LimitedAccess::AtomicStorageBase {
	mutable Boolean atomicFlag = false;
	mutable bool atomicActiveFlag = true;

protected:
	virtual ~AtomicStorageBase() { atomicActiveFlag = false; }

public:
	void atomicAcquire() const noexcept {
		bool expected;
		while (expected = false, atomicActiveFlag && !atomicFlag.compare_exchange_strong(expected, true));
	}

	void atomicRelease() const noexcept {
		bool expected;
		while (expected = true, atomicActiveFlag && !atomicFlag.compare_exchange_strong(expected, false));
	}

	bool atomicActive() const noexcept { return atomicActiveFlag; }
	bool atomicAcquired() const noexcept { return atomicFlag; }
	AtomicGuard atomicGuard() const noexcept { return AtomicGuard(this); }
};

/**
 * @brief 给一个类的对象提供多线程原子保护。可以继承。
 */
struct AtomicStorage : $LimitedAccess::AtomicStorageBase {
	AtomicStorage() = default;
	~AtomicStorage() noexcept override = default;
};

inline AtomicGuard::AtomicGuard(const $LimitedAccess::AtomicStorageBase* storage): storage(storage) {
	requireNonnull(storage);
	storage->atomicAcquire();
}

inline AtomicGuard::~AtomicGuard() noexcept { if (storage && storage->atomicActive() && storage->atomicAcquired()) storage->atomicRelease(); }
