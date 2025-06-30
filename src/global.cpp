//
// Created by EmsiaetKadosh on 25-6-23.
//

#include "global.hpp"

#include "game\Game.h"
#include "interact\InteractManager.h"
#include "render\Pose.hpp"

#ifdef __CARLBEKS_USE_DX__
#include "render\dx\direct.hpp"
using RendererType = DirectX12Renderer;
#else
#include "render\gdi\Renderer.hpp"
#include "render\gdi\texture.hpp"
#include "render\gdi\font.hpp"
using RendererType = GdiRenderer;
using TextureManagerType = GdiTextureManager;
using FontManagerType = GdiFontManager;
#endif

namespace $LimitedAccess {
	struct MemoryManager {
		struct MemoryInfo {
			const String msg;
			std::size_t size;
		};

		Map<void*, MemoryInfo> allocated {};
		std::atomic_bool acquiring = false;
		MemoryManager() noexcept { initializerChecker.registerModule(L"MemoryManager"); }
	};

	struct Release {
		Release() noexcept { initializerChecker.registerModule(L"Releaser"); }

		~Release() {
			Logger.put(L"--------- Last Check ---------");
			for (const auto& [addr, info] : memoryManager.allocated) Logger.print(L"  using", addr, info.size, L"B", info.msg);
			std::atomic_thread_fence(std::memory_order_acquire);
			Logger.put(L"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
		}
	};
}

void InitializerChecker::registerModule(const String& name) noexcept(false) {
	if (modules.contains(name)) throw RuntimeException(L"Module [[[" + name + L"]]] already registered.");
	modules.emplace(name);
	if (modules.contains(L"Logger")) Logger.info(L"Module [[[" + name + L"]]] registered.");
}

InitializerChecker& InitializerChecker::requiredModule(const String& current, const String& requiring) noexcept(false) {
	if (modules.contains(current)) return *this;
	if (modules.contains(requiring)) return *this;
	Logger.fatal(L"Unregistered module [[[" + requiring + L"]]] is required by [[[" + current + L"]]].");
	throw RuntimeException(L"Unregistered module [[[" + requiring + L"]]] is required by [[[" + current + L"]]].");
}

InitializerChecker initializerCheckerStatic = InitializerChecker();
$LimitedAccess::MemoryManager memoryManagerStatic;
PublicLogger loggerStatic = PublicLogger(L"Main");
$LimitedAccess::Release releaseStatic;
GarbageCollector gcStatic;
ModelManager modelManagerStatic;
TextureManagerType textureManagerStatic;
Game gameStatic;
RendererType rendererStatic;
FontManagerType fontManagerStatic = FontManagerType(&rendererStatic);
InteractManager interactManagerStatic;
InteractSettings interactSettingsStatic;

IRenderer& staticRenderer() {
	static IRenderer& r = rendererStatic.postInitialize();
	return r;
}

InitializerChecker& instanceInitializerChecker() { return initializerCheckerStatic; }
$LimitedAccess::MemoryManager& $LimitedAccess::instanceMemoryManager() { return memoryManagerStatic; }
PublicLogger& instancePublicLogger() { return loggerStatic; }
GarbageCollector& instanceGarbageCollector() { return gcStatic; }
ModelManager& instanceModelManager() { return modelManagerStatic; }
ITextureManager& instanceTextureManager() { return textureManagerStatic; }
Game& instanceGame() { return gameStatic; }
IRenderer& instanceRenderer() { return staticRenderer(); }
IFontManager& instanceFontManager() { return fontManagerStatic; }
InteractManager& instanceInteractManager() { return interactManagerStatic; }
InteractSettings& instanceInteractSettings() { return interactSettingsStatic; }


namespace $LimitedAccess {
#if defined __CARLBEKS_DEBUG__ || defined __CARLBEKS_MEMORY__
	inline void printAllocate(void* value, const size_t size, const String& msg) {
#if __CARLBEKS_MEMORY__ > 2
		const String str = L"alloc   " + ptrtow(reinterpret_cast<QWORD>(value)) + L" " + std::to_wstring(size) + String(L"B ") + msg;
		Logger.log(str);
#endif
	}

	inline void printDeallocate(void* value, const size_t size, const String& msg) {
#if __CARLBEKS_MEMORY__ > 3
		const String str = L"dealloc " + ptrtow(reinterpret_cast<QWORD>(value)) + L" " + std::to_wstring(size) + String(L"B ") + msg;
		Logger.log(str);
#endif
	}
#endif

	inline void printDeallocateWarning(void* value, const String& msg) {
		const String str = L"dealloc " + ptrtow(reinterpret_cast<QWORD>(value)) + L": " + msg;
		Logger.error(str);
	}

	void registerAllocate(void* value, std::string&& type, const String& msg, const QWORD size) {
		bool expect = false;
		while (!memoryManager.acquiring.compare_exchange_strong(expect, true)) expect = false;
		const auto& k = memoryManager.allocated.emplace(value, MemoryManager::MemoryInfo { L"[" + atow(type.c_str()) + L"]" + msg, size }).first;
		printAllocate(value, k->second.size, k->second.msg);
		memoryManager.acquiring.store(false);
	}

	void unregisterDeallocate(void* value, const String& stack) {
		bool expect = false;
		while (!memoryManager.acquiring.compare_exchange_strong(expect, true)) expect = false;
#if __CARLBEKS_MEMORY__ > 2
		const MemoryManager::MemoryInfo* info = nullptr;
		if (memoryManager.allocated.contains(value)) info = &memoryManager.allocated.at(value);
		printDeallocate(value, info ? info->size : 0, info ? info->msg : L"???");
#endif
		if (value) if (!memoryManager.allocated.erase(value)) printDeallocateWarning(value, stack.empty() ? L"value not recorded" : L"value not recorded\n    " + stack);
		memoryManager.acquiring.store(false);
	}
}
