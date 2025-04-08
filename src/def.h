//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#define __CARLBEKS_DEBUG__
#define __CARLBEKS_MEMORY__ 2

#pragma warning(disable: 4819)

#include <typeinfo>
#include <functional>
#include <thread>
#include <iostream>
#include <list>
#include <string>
#include <map>
#include <chrono>
#include <atomic>
#include <fstream>
#include <sstream>
#include <cmath>
#include <set>
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
template <typename K, typename V, typename Cmp = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
using Map = std::map<K, V, Cmp, Alloc>;
template <typename T, typename Comparator = std::less<T>, typename Allocator = std::allocator<T>>
using Set = std::set<T, Comparator, Allocator>;
template <typename T, typename Allocator = std::allocator<T>>
using List = std::list<T, Allocator>;
template <typename T, typename Allocator = std::allocator<T>>
using Vector = std::vector<T, Allocator>;
template <typename F>
using Function = std::function<F>;
template <typename T>
using Atomic = std::atomic<T>;

#define Success() { return 0; }
#define Failed() { return 1; }
#define Error() { return -1; }
#define Comment(PARAMS) /##/ PARAMS
#define SameAs(PARAMS) Comment(PARAMS)
#define pass // ((void) 0)

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

// DirectX
// #include <d3d12.h> // 估计是版本过于老旧了，用不了一点
#include "include\include\directx\d3d12.h"
#include "include\include\directx\d3dx12.h"
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#ifdef __CARLBEKS_DEBUG__
#include <dxgidebug.h>
#endif
#include <wrl.h>

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
// DirectX
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

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
	} inline gcRelease_LoggerRelease_memoryManagerRelease;

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
namespace $LimitedAccess {
	void printAllocate(void* value, std::size_t size, const String&);
	void printDeallocate(void* value, std::size_t size, const String&);
	void printDeallocateWarning(void* value, const String& msg);
}

[[noreturn]] void unreachable() noexcept(false);
extern String atow(const char* chars);

template <typename T>
T* allocatedFor$(T* value, const String& msg = L"", std::size_t size = sizeof(T)) {
	requireNonnull(value);
	bool expect = false;
	while (!$LimitedAccess::memoryManager.acquiring.compare_exchange_strong(expect, true)) expect = false;
	const auto& k = $LimitedAccess::memoryManager.allocated.emplace(value, $LimitedAccess::MemoryManager::MemoryInfo{L"[" + atow(typeid(T).name()) + L"] " + msg, size}).first;
#if __CARLBEKS_MEMORY__ > 2
	$LimitedUse::printAllocate(value, k->second.size, k->second.msg);
#endif
	$LimitedAccess::memoryManager.acquiring.store(false);
	return value;
}

template <typename T>
T* deallocating$(T* value, const String& stack) {
	bool expect = false;
	while (!$LimitedAccess::memoryManager.acquiring.compare_exchange_strong(expect, true)) expect = false;
#if __CARLBEKS_MEMORY__ > 2
	const $LimitedUse::MemoryManager::MemoryInfo* info = nullptr;
	if ($LimitedUse::memoryManager.allocated.contains(value)) info = &$LimitedUse::memoryManager.allocated.at(value);
	$LimitedUse::printDeallocate(value, info ? info->size : 0, info ? info->msg : L"???");
#endif
	if (value) if (!$LimitedAccess::memoryManager.allocated.erase(value)) $LimitedAccess::printDeallocateWarning(value, L"value not recorded" + stack);
	$LimitedAccess::memoryManager.acquiring.store(false);
	return value;
}

#ifndef __FUNCSIG__
#define __FUNCSIG__ atow(__func__)
#define __FUNCSIG__PACK__ + __FUNCSIG__ +
#else
#define __FUNCSIG__PACK__ __FUNCSIG__
#endif

#if __CARLBEKS_MEMORY__ > 3
#define allocatedFor(val, ...) allocatedFor$(val, L"\n    From " __FUNCSIG__PACK__ L"\n    At   " __FILE__ ":" _STL_STRINGIZE(__LINE__) __VA_OPT__(,) __VA_ARGS__)
#else
#define allocatedFor(val, ...) allocatedFor$(val, L"" __VA_OPT__(,) __VA_ARGS__)
#endif
#if __CARLBEKS_MEMORY__ > 1
#define deallocating(val) deallocating$(val, L"\n    From " __FUNCSIG__PACK__ L"\n    At   " __FILE__ ":" _STL_STRINGIZE(__LINE__))
#else
#define deallocating(val) deallocating$(val)
#endif

#else
#define allocatedFor(val, ...) val
#define deallocating(val) val
#endif

template <TypeName Base>
class ObjectHolder {
	Base* value;
	bool hasValue;
	char padding[7]{};

public:
	/**
	 * 用于延迟初始化。
	 */
	ObjectHolder() : value(nullptr), hasValue(false) {}

	ObjectHolder(Base* value) : value(value), hasValue(false) {}

	template <NewCopyable T> requires (std::is_base_of_v<Base, T> || std::is_same_v<Base, T>) && TypeName<T>
	ObjectHolder(const T& value) : value(allocatedFor(new T(value))), hasValue(true) {}

	template <NewMoveable T> requires (std::is_base_of_v<Base, T> || std::is_same_v<Base, T>) && TypeName<T>
	ObjectHolder(T&& value) : value(allocatedFor(new T(std::forward<T>(value)))), hasValue(true) {}

	ObjectHolder(const ObjectHolder& other) noexcept: value(other.value), hasValue(false) {}

	ObjectHolder(ObjectHolder&& other) noexcept: value(other.value), hasValue(other.hasValue) {
		other.value = nullptr;
		other.hasValue = false;
	}

	template <NewCopyable T> requires std::is_base_of_v<Base, T> && TypeName<T>
	void set(const T& value);

	template <NewMoveable T> requires std::is_base_of_v<Base, T> && TypeName<T>
	void set(T&& value);

	template <TypeName T, typename... ConstructorParams>
	T& allocate(ConstructorParams&&... params) {
		if (hasValue) delete deallocating(value);
		value = allocatedFor(new T(std::forward<ConstructorParams>(params)...));
		return *value;
	}

	~ObjectHolder() {
		if (hasValue) delete deallocating(value);
		value = nullptr;
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
	ObjectHolder<T> referenceof(const T& other) {
		ObjectHolder ret{};
		ret.value = &other;
		return ret;
	}
};

template <TypeName Base>
class SynchronizedHolder {
	mutable Base* newValue = nullptr;
	mutable Base* value = nullptr;
	mutable bool isOk = false;

public:
	SynchronizedHolder() = default;

	~SynchronizedHolder() {
		if (newValue == value) { if (value) delete deallocating(value); }
		else {
			if (newValue) delete deallocating(newValue);
			if (value) delete deallocating(value);
		}
		newValue = nullptr;
		value = nullptr;
	}


	template <NewCopyable T> requires std::is_base_of_v<Base, T> && TypeName<T>
	void setNew(const Base& other) noexcept {
		isOk = false;
		if (newValue && newValue != value) deleteNew();
		newValue = allocatedFor(new T(other));
	}

	template <NewMoveable T> requires std::is_base_of_v<Base, T> && TypeName<T>
	void setNew(T&& val) noexcept {
		isOk = false;
		if (newValue && newValue != value) deleteNew();
		newValue = allocatedFor(new T(std::forward<T>(val)));
	}

	void ok() const noexcept { isOk = true; }

	Base& get() const noexcept(false) {
		requireNonnull(value);
		return *value;
	}

	Base& getNew() const noexcept(false) {
		requireNonnull(newValue); // 用于抛错
		return *newValue;
	}

	Base* ptr() const noexcept { return value; }
	Base* ptrNew() const noexcept { return newValue; }
	Base* ptrs() const noexcept { return newValue ? newValue : value; }

	void async() const noexcept {
		if (newValue == value) return;
		if (!isOk) return;
		Base* nuv = newValue;
		newValue = nullptr;
		if (nuv) {
			if (value && value != nuv) deleteOld();
			value = nuv;
		}
	}

private:
	void deleteOld() const noexcept {
		delete deallocating(value);
		value = nullptr;
	}

	void deleteNew() const noexcept {
		delete deallocating(newValue);
		newValue = nullptr;
	}
};
