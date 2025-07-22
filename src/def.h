//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

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
template <typename K, typename V, typename Cmp = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>> using Map = std::map<K, V, Cmp, Alloc>;
template <typename T, typename Comparator = std::less<T>, typename Allocator = std::allocator<T>> using Set = std::set<T, Comparator, Allocator>;
template <typename T, typename Allocator = std::allocator<T>> using List = std::list<T, Allocator>;
template <typename T, typename Allocator = std::allocator<T>> using Vector = std::vector<T, Allocator>;
template <typename T, typename Allocator = std::allocator<T>> using Deque = std::deque<T, Allocator>;
template <typename F> using Function = std::function<F>;
template <typename T> using Atomic = std::atomic<T>;
template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = std::allocator<std::pair<const Key, T>>> using HashMap = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;

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
template <typename T>
concept Nonabstract = !std::is_abstract_v<T>;

void requireNonnull(const void* value) noexcept(false);
void checkAllocation(const void* value) noexcept(false);
[[noreturn]] void unreachable() noexcept(false);

namespace $LimitedAccess {
	template <typename T> struct Annotations {
		[[carlbeks::nonnull]] T* nonnull_ptr;
		[[carlbeks::nullable]] T* nullable_ptr;
		[[carlbeks::defineat("str: file")]] T any_declaration;
		[[carlbeks::predecl]] T any_declaration_without_definition;
		[[carlbeks::optimize]] T any_function_can_optimize;
	};
}

#include "global.hpp"

template <TypeName Base> class Container {
	template <TypeName T> friend class ::Container;

	Base* value;
	bool hasValue;
	char padding[7] {};

	template <typename... Args> Base* create(Args&&... args) {
		if constexpr (std::is_abstract_v<Base>) return nullptr;
		else return new Base(std::forward<Args>(args)...);
	}

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

	template <typename T, typename... Args> requires std::is_base_of_v<Base, T> || std::is_same_v<Base, T>
	Container(T** out_ptr, Args&&... args) : value(allocatedFor(*out_ptr = new T(std::forward<Args>(args)...))), hasValue(true) {}

	template <typename... Args> requires Nonabstract<Base> && (sizeof...(Args) > 1 || !((std::is_same_v<std::decay_t<Base>, Args>) || ...)) // 约束非抽象Base + 非移动/复制构造，才能使用这个函数
	Container(Args&&... args) : value(allocatedFor(create(std::forward<Args>(args)...))), hasValue(true) {}

	~Container() {
		if (hasValue) delete deallocating(value);
		value = nullptr;
	}

	template <TypeName T, typename... ConstructorParams> T& allocate(ConstructorParams&&... params) {
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

	template <typename T> static Container<T> referenceof(const T& other) {
		Container ret {};
		ret.value = &other;
		return ret;
	}
};

#define __CARLBEKS_BINARY__(op) \
	template <typename U> requires requires { (*ref) op std::declval<U>(); } \
	decltype(auto) operator op (U&& u) noexcept(false) { return requireNonnull(ref), ((*ref) op std::forward<U>(u)); }

#define __CARLBEKS_UNARY__(op) \
	decltype(auto) operator op () noexcept(false) requires requires { (op (*ref)); } { return requireNonnull(ref), (op (*ref)); }

template <typename T> class Reference {
	T* ref;

public:
	Reference(nullptr_t) : ref(nullptr) {}
	Reference(T& ref) : ref(&ref) {}
	Reference(const Reference& other) noexcept : ref(other.ref) {}
	Reference(Reference&& other) noexcept : ref(other.ref) {}
	Reference& operator=(const Reference& other) noexcept { return ref = other.ref, *this; }
	Reference& operator=(Reference&& other) noexcept { return ref = other.ref, *this; }
	~Reference() = default;

	void rebind(T& ref) noexcept { this->ref = &ref; }
	operator T&() noexcept { return *ref; }

	__CARLBEKS_UNARY__(+)
	__CARLBEKS_BINARY__(+)
	__CARLBEKS_UNARY__(-)
	__CARLBEKS_BINARY__(-)
	__CARLBEKS_UNARY__(*)
	__CARLBEKS_BINARY__(*)
	__CARLBEKS_BINARY__(/)
	__CARLBEKS_BINARY__(%)
	__CARLBEKS_BINARY__(^)
	__CARLBEKS_BINARY__(&)
	__CARLBEKS_BINARY__(|)
	__CARLBEKS_UNARY__(~)
	__CARLBEKS_UNARY__(!)
	__CARLBEKS_BINARY__(=)
	__CARLBEKS_BINARY__(<)
	__CARLBEKS_BINARY__(>)
	__CARLBEKS_BINARY__(+=)
	__CARLBEKS_BINARY__(-=)
	__CARLBEKS_BINARY__(*=)
	__CARLBEKS_BINARY__(/=)
	__CARLBEKS_BINARY__(%=)
	__CARLBEKS_BINARY__(^=)
	__CARLBEKS_BINARY__(&=)
	__CARLBEKS_BINARY__(|=)
	__CARLBEKS_BINARY__(<<)
	__CARLBEKS_BINARY__(>>)
	__CARLBEKS_BINARY__(<<=)
	__CARLBEKS_BINARY__(>>=)
	__CARLBEKS_BINARY__(==)
	__CARLBEKS_BINARY__(!=)
	__CARLBEKS_BINARY__(<=)
	__CARLBEKS_BINARY__(>=)
	__CARLBEKS_BINARY__(<=>)
	__CARLBEKS_UNARY__(++)
	__CARLBEKS_UNARY__(--)
	__CARLBEKS_BINARY__(->*)

	auto operator->() noexcept(false) requires requires { *ref; } {
		requireNonnull(ref);
		if constexpr (requires { ref->operator->(); }) return ref->operator->();
		else return ref;
	}

	template <typename... Args> requires requires { ref->operator()(std::declval<Args>()...); }
	auto operator()(Args&&... args) noexcept(false) {
		requireNonnull(ref);
		return ref->operator()(std::forward<Args>(args)...);
	}

	template <typename... Args> requires requires { ref->operator[]((std::declval<Args>(), ...)); }
	auto operator[](Args&&... args) noexcept(false) {
		requireNonnull(ref);
		return ref->operator[](std::forward<Args>(args)...);
	}

	// Special Operators

	template <typename U> requires requires { ref->operator&&(std::declval<U>()); } // 强制仅显式重载，否则允许短路
	auto operator&&(U&& u) noexcept(false) {
		static_assert(!std::is_same_v<std::decay_t<U>, bool>, "Overloaded operator&& loses short-circuiting with bool.");
		return requireNonnull(ref), ref->operator&&(std::forward<U>(u));
	}

	template <typename U> requires requires { ref->operator||(std::declval<U>()); } // 强制仅显式重载，否则允许短路
	auto operator||(U&& u) noexcept(false) {
		static_assert(!std::is_same_v<std::decay_t<U>, bool>, "Overloaded operator|| loses short-circuiting with bool.");
		return requireNonnull(ref), ref->operator||(std::forward<U>(u));
	}


	template <typename U> requires requires { ref->operator,(std::declval<U>()); } // 强制仅显式重载，否则内置定序
	auto operator,(U&& u) noexcept(false) { return requireNonnull(ref), ref->operator,(std::forward<U>(u)); }

	auto operator&() noexcept(false) {
		requireNonnull(ref);
		if constexpr (requires { ref->operator&(); }) return ref->operator&();
		else return ref;
	}

	decltype(auto) operator++(int) noexcept(false) requires requires { (*ref)++; } { return requireNonnull(ref), (*ref)++; }
	decltype(auto) operator--(int) noexcept(false) requires requires { (*ref)--; } { return requireNonnull(ref), (*ref)--; }
};

#undef __CARLBEKS_UNARY__
#undef __CARLBEKS_BINARY__
