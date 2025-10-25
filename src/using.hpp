
// ReSharper disable once CppMissingIncludeGuard

#include <Windows.h>

#ifndef HBP_USING
#define HBP_USING

using wchar = wchar_t;

using FontStyle = int;
using FontID = unsigned short;

#endif

#if defined(_STRING_) && !defined(HBP_STRING)
#define HBP_STRING
using String = std::wstring;
#endif

#if defined(_VECTOR_) && !defined(HBP_VECTOR)
#define HBP_VECTOR
template <typename T, typename Alloc = std::allocator<T>> using Vector = std::vector<T, Alloc>;
#endif

#if defined(_DEQUE_) && !defined(HBP_DEQUE)
#define HBP_DEQUE
template<typename T, typename Alloc = std::allocator<T>>
using Deque = std::deque<T, Alloc>;
#endif

#if defined(_LIST_) && !defined(HBP_LIST)
#define HBP_LIST
template<typename T, typename Alloc = std::allocator<T>> using List = std::list<T, Alloc>;
#endif

#if defined(_MAP_) && !defined(HBP_MAP)
#define HBP_MAP
template <typename K, typename V, typename Compare = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>> using Map = std::map<K, V, Compare, Alloc>;
#endif

#if defined(_SET_) && !defined(HBP_SET)
#define HBP_SET
template <typename K, typename Compare = std::less<K>, typename Alloc = std::allocator<K>> using Set = std::set<K, Compare, Alloc>;
#endif

#if defined(_WRL_H_) && !defined(HBP_WRL_H)
#define HBP_WRL_H
template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
#endif

#if defined(_STACKTRACE_) && !defined(HBP_STACKTRACE)
#define HBP_STACKTRACE
using Stacktrace = std::stacktrace;
#endif

#if defined(_THREAD_) && !defined(HBP_THREAD)
#define HBP_THREAD
using Thread = std::thread;
#endif

#if defined(_CHRONO_) && !defined(HBP_CHRONO)
#define HBP_CHRONO
using Chronono = std::chrono::high_resolution_clock;
using SystemClock = std::chrono::system_clock;
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using TimePeriod = Clock::duration;
#endif
