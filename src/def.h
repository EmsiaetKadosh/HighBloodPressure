//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#define __CARLBEKS_DEBUG__
#define __CARLBEKS_MEMORY__ 2

#pragma warning(disable: 4819)

#include <typeinfo>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream> // has string
#include <list>
#include <map>
#include <set>
#include <deque>
#include <cmath>
#include <filesystem>
#include <random>
#include <stacktrace>

using wchar = wchar_t;
using QWORD = unsigned long long int;
using String = std::wstring;
using Thread = std::thread;
using Time = std::chrono::time_point<std::chrono::system_clock>;
using NanoDuration = std::chrono::duration<long long, std::nano>;
using Boolean = std::atomic_bool;
using Mutex = std::mutex;
template <typename K, typename V, typename Cmp = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
using Map = std::map<K, V, Cmp, Alloc>;
template <typename T, typename Comparator = std::less<T>, typename Allocator = std::allocator<T>>
using Set = std::set<T, Comparator, Allocator>;
template <typename T, typename Allocator = std::allocator<T>>
using List = std::list<T, Allocator>;
template <typename T, typename Allocator = std::allocator<T>>
using Vector = std::vector<T, Allocator>;
template <typename T, typename Allocator = std::allocator<T>>
using Deque = std::deque<T, Allocator>;
template <typename F>
using Function = std::function<F>;
template <typename T>
using Atomic = std::atomic<T>;
template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = std::allocator<std::pair<const Key, T>>>
using HashMap = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;

#define Success() { return 0; }
#define Failed() { return 1; }
#define Error() { return -1; }
#define Comment(PARAMS) /##/ PARAMS
#define SameAs(PARAMS) Comment(PARAMS)
#define pass // ((void) 0)
#define assert_dynamic_cast static_cast

//NOLINTNEXTLINE(*-reserved-identifier)
#define _WINSOCKAPI_ /* 防止winsock.h被引入。winsock.h和winsock2.h冲突。 */
#if false
#include <WinSock2.h>
#endif

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Windowsx.h>
#include <minwindef.h>
#include <windef.h>
#include <wingdi.h>
#include <WinUser.h>
#include <Uxtheme.h>
#include <dwmapi.h>

#define WM_APP_LBUTTONUP (WM_APP + 1)
#define WM_APP_MBUTTONDOWN (WM_APP + 2)
#define WM_APP_GAMESTART (WM_APP + 3)
#define WM_APP_EXITSIZEMOVE (WM_APP + 4)
#define WM_APP_REQUESTHDC (WM_APP + 5)

#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Uxtheme.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "imm32.lib")

#include "warnings.h"

template <typename T>
concept Copyable = requires(const T& t) { T(t); };
template <typename T>
concept NewCopyable = requires(const T& t) { delete new T(t); };
template <typename T>
concept Moveable = requires(T&& t) { T(std::move(t)); };
template <typename T>
concept NewMoveable = requires(T&& t) { delete new T(std::move(t)); };
template <typename T>
concept NonreferenceType = !std::is_reference_v<T>;
template <typename T>
concept NonpointerType = !std::is_pointer_v<T>;
template <typename T>
concept ReferenceType = std::is_reference_v<T>;
template <typename T>
concept PointerType = std::is_pointer_v<T>;
template <typename T>
concept TypeName = NonreferenceType<T> && NonpointerType<T>;

namespace $LimitedAccess {
	struct Release {
		Release() = default;

		~Release();
	} extern gcRelease_LoggerRelease_memoryManagerRelease;

	struct MemoryManager {
		struct MemoryInfo {
			const String msg;
			std::size_t size;
		};

		Map<void*, MemoryInfo> allocated{};
		std::atomic_bool acquiring = false;

		constexpr MemoryManager() noexcept = default;
	} inline& [[carlbeks::releasedat("def.cpp")]] memoryManager = *new MemoryManager;
}

void requireNonnull(const void* value) noexcept(false);
void checkAllocation(const void* value) noexcept(false);
inline String ptrtow(QWORD value);

#if defined __CARLBEKS_DEBUG__ || defined __CARLBEKS_MEMORY__
[[noreturn]] void unreachable() noexcept(false);
extern String atow(const char* chars);

namespace $LimitedAccess {
#if __CARLBEKS_MEMORY__ > 2
	void printAllocate(void* value, std::size_t size, const String&);
#endif
#if __CARLBEKS_MEMORY__ > 3
	void printDeallocate(void* value, std::size_t size, const String&);
#endif
	void printDeallocateWarning(void* value, const String& msg);

	template <typename T>
	T* allocatedFor$(T* value, const String& msg = L"", std::size_t size = sizeof(T)) {
		requireNonnull(value);
		bool expect = false;
		while (!memoryManager.acquiring.compare_exchange_strong(expect, true)) expect = false;
		const auto& k = memoryManager.allocated.emplace(value, MemoryManager::MemoryInfo{L"[" + atow(typeid(T).name()) + L"] " + msg, size}).first;
#if __CARLBEKS_MEMORY__ > 2
		printAllocate(value, k->second.size, k->second.msg);
#endif
		memoryManager.acquiring.store(false);
		return value;
	}

	template <typename T>
	T* deallocating$(
		T* value
#if __CARLBEKS_MEMORY__ > 1
		, const String& stack
#endif
	) {
		bool expect = false;
		while (!memoryManager.acquiring.compare_exchange_strong(expect, true)) expect = false;
#if __CARLBEKS_MEMORY__ > 2
		const MemoryManager::MemoryInfo* info = nullptr;
		if (memoryManager.allocated.contains(value)) info = &memoryManager.allocated.at(value);
		printDeallocate(value, info ? info->size : 0, info ? info->msg : L"???");
#endif
		if (value)
			if (!memoryManager.allocated.erase(value))
				printDeallocateWarning(
					value,
#if __CARLBEKS_MEMORY__ > 1
					stack.empty() ? L"value not recorded" : L"value not recorded\n    " + stack
#else
					L"value not recorded"
#endif
				);
		memoryManager.acquiring.store(false);
		return value;
	}
}

#ifndef __FUNCSIG__
#define __FUNCSIG__ atow(__func__)
#define __FUNCSIG__PACK__ + __FUNCSIG__ +
#else
#define __FUNCSIG__PACK__ __FUNCSIG__
#endif

#if __CARLBEKS_MEMORY__ > 3
#define allocatedFor(val, ...) $LimitedAccess::allocatedFor$(val, L"From " __FUNCSIG__PACK__ L"\n    At   " __FILE__ ":" _STL_STRINGIZE(__LINE__) __VA_OPT__(,) __VA_ARGS__)
#else
#define allocatedFor(val, ...) $LimitedAccess::allocatedFor$(val, L"" __VA_OPT__(,) __VA_ARGS__)
#endif
#if __CARLBEKS_MEMORY__ > 1
#define deallocating(val) $LimitedAccess::deallocating$(val, L"From " __FUNCSIG__PACK__ L"\n    At   " __FILE__ ":" _STL_STRINGIZE(__LINE__))
#define deallocating_message(val, msg) $LimitedAccess::deallocating$(val, msg)
#else
#define deallocating(val) $LimitedAccess::deallocating$(val)
#define deallocating_message(val, msg) deallocating(val)
#endif

#else
#define allocatedFor(val, ...) val
#define deallocating(val) val
#endif

namespace $LimitedAccess {
	template <typename T>
	struct Annotations {
		[[carlbeks::nonnull]] T* nonnull_ptr;
		[[carlbeks::nullable]] T* nullable_ptr;
		[[carlbeks::defineat("str: file")]] T any_declaration;
		[[carlbeks::predecl]] T any_declaration_without_definition;
		[[carlbeks::optimize]] T any_function_can_optimize;
	};
}

template <TypeName Base>
class Container {
	Base* value;
	bool hasValue;
	char padding[7]{};

public:
	/**
	 * 用于延迟初始化。
	 */
	Container() : value(nullptr), hasValue(false) {}
	Container(Base* value) : value(value), hasValue(false) {}
	Container(const Container& other) noexcept: value(other.value), hasValue(false) {}
	Container(Container&& other) noexcept: value(other.value), hasValue(other.hasValue) { other.value = nullptr, other.hasValue = false; }

	template <NewCopyable T> requires (std::is_base_of_v<Base, T> || std::is_same_v<Base, T>) && TypeName<T>
	Container(const T& value) : value(allocatedFor(new T(value))), hasValue(true) {}

	template <NewMoveable T> requires (std::is_base_of_v<Base, T> || std::is_same_v<Base, T>) && TypeName<T>
	Container(T&& value) : value(allocatedFor(new T(std::forward<T>(value)))), hasValue(true) {}

	template <TypeName T> requires std::is_base_of_v<Base, T> || std::is_same_v<Base, T>
	Container(Container<T>&& other) noexcept : value(other.value), hasValue(other.hasValue) { other.value = nullptr, other.hasValue = false; }

	template <typename ...Args>
	Container(Args&&... args) : value(allocatedFor(new Base(std::forward<Args>(args)...))), hasValue(true) {}

	~Container() {
		if (hasValue) delete deallocating(value);
		value = nullptr;
	}

	template <TypeName T, typename... ConstructorParams>
	T& allocate(ConstructorParams&&... params) {
		if (hasValue) delete deallocating(value);
		value = allocatedFor(new T(std::forward<ConstructorParams>(params)...));
		return *value;
	}

	[[nodiscard]] Base* operator->() noexcept(false) {
		requireNonnull(value);
		return value;
	}

	[[nodiscard]] const Base* operator->() const noexcept(false) {
		requireNonnull(value);
		return value;
	}

	[[nodiscard]] Base& operator*() noexcept(false) {
		requireNonnull(value);
		return *value;
	}

	[[nodiscard]] const Base& operator*() const noexcept(false) {
		requireNonnull(value);
		return *value;
	}

	[[nodiscard]] Base& get() noexcept(false) {
		requireNonnull(value);
		return *value;
	}

	[[nodiscard]] const Base& get() const noexcept(false) {
		requireNonnull(value);
		return *value;
	}

	[[nodiscard]] operator Base*() const noexcept(false) { return value; }
	Base* ptr() const noexcept { return value; }
	operator bool() const noexcept { return value; }
	bool operator!() const noexcept { return value == nullptr; }
	[[nodiscard]] bool isManager() const noexcept { return hasValue; }

	template <typename T>
	static Container<T> referenceof(const T& other) {
		Container ret{};
		ret.value = &other;
		return ret;
	}
};
