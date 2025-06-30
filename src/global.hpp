//
// Created by EmsiaetKadosh on 25-6-23.
//

#pragma once

#define __CARLBEKS_DEBUG__
#define __CARLBEKS_MEMORY__ 2

#include <string>
#include <set>

#include "warnings.h"

class InitializerChecker {
	template<typename T> using Set = std::set<T>;
	using String = std::wstring;

	Set<String> modules {};
public:
	InitializerChecker() = default;
	void registerModule(const String& name) noexcept(false);
	InitializerChecker& requiredModule(const String& current, const String& requiring) noexcept(false);
};

InitializerChecker& instanceInitializerChecker();
class PublicLogger& instancePublicLogger();
class GarbageCollector& instanceGarbageCollector();
class ModelManager& instanceModelManager();
class ITextureManager& instanceTextureManager();
class Game& instanceGame();
class IRenderer& instanceRenderer();
class IFontManager& instanceFontManager();
class InteractManager& instanceInteractManager();
class InteractSettings& instanceInteractSettings();

inline InitializerChecker& initializerChecker = instanceInitializerChecker();
inline PublicLogger& Logger = instancePublicLogger();
inline GarbageCollector& gc = instanceGarbageCollector();
inline ModelManager& modelManager = instanceModelManager();
inline ITextureManager& textureManager = instanceTextureManager();
inline Game& game = instanceGame();
inline IRenderer& renderer = instanceRenderer();
inline IFontManager& fontManager = instanceFontManager();
inline InteractManager& interactManager = instanceInteractManager();
inline InteractSettings& interactSettings = instanceInteractSettings();

namespace $LimitedAccess {
	using String = std::wstring;
	using QWORD = unsigned long long;

	struct MemoryManager& instanceMemoryManager();
	inline MemoryManager& memoryManager = instanceMemoryManager();
	void registerAllocate(void* value, std::string&& type,  const String& msg, QWORD size);
	void unregisterDeallocate(void* value, const String& stack);
	inline void printAllocate(void* value, std::size_t size, const String&);
	inline void printDeallocate(void* value, std::size_t size, const String&);
	inline void printDeallocateWarning(void* value, const String& msg);

	template <typename T> T* allocatedFor$(T* value, const String& msg = L"", std::size_t size = sizeof(T)) {
		requireNonnull(value);
		registerAllocate(value, typeid(T).name(), msg, size);
		return value;
	}

	template <typename T> T* deallocating$(T* value, const String& stack = L"") {
		unregisterDeallocate(value, stack);
		return value;
	}
}

#if defined __CARLBEKS_DEBUG__ || defined __CARLBEKS_MEMORY__

#if __CARLBEKS_MEMORY__ > 3
#define allocatedFor(val, ...) $LimitedAccess::allocatedFor$(val, L"From " __FUNCSIG__PACK__ L"\n    At   " __FILE__ ":" _STL_STRINGIZE(__LINE__) __VA_OPT__(,) __VA_ARGS__)
#else
#define allocatedFor(val, ...) $LimitedAccess::allocatedFor$(val, L"" __VA_OPT__(,) __VA_ARGS__)
#endif // ^ allocatedFor

#if __CARLBEKS_MEMORY__ > 1
#define deallocating(val) $LimitedAccess::deallocating$(val, L"From " __FUNCSIG__ "\n    At   " __FILE__ ":" _STL_STRINGIZE(__LINE__))
#define deallocating_message(val, msg) $LimitedAccess::deallocating$(val, msg)
#else
#define deallocating(val) $LimitedAccess::deallocating$(val)
#define deallocating_message(val, msg) deallocating(val)
#endif // ^ deallocating

#else
#define allocatedFor(val, ...) val
#define deallocating(val) val
#endif
