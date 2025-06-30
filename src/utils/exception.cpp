//
// Created by EmsiaetKadosh on 25-1-22.
//

#pragma once

#include "..\def.h"
#include "exception.h"
#include "..\utils\utils.h"
#include "..\game\Game.h"
#include "..\hbp.h"
#include "..\game\world\World.h"

static AtomicStorage outStreamLock = false;

PublicLogger& LoggableString::trace() noexcept { return notLogged = false, logger->trace(str); }
PublicLogger& LoggableString::debug() noexcept { return notLogged = false, logger->debug(str); }
PublicLogger& LoggableString::log() noexcept { return notLogged = false, logger->log(str); }
PublicLogger& LoggableString::info() noexcept { return notLogged = false, logger->info(str); }
PublicLogger& LoggableString::warn() noexcept { return notLogged = false, logger->warn(str); }
PublicLogger& LoggableString::error() noexcept { return notLogged = false, logger->error(str); }
PublicLogger& LoggableString::fatal() noexcept { return notLogged = false, logger->fatal(str); }

String PublicLogger::build(const String& msg, const String& type) const {
	return
		L"GT-" + qwtowb10(game.getTick(), 8) +
		(game.isReady() ? L" WT-" + qwtowb10(game.getWorldManager().getTick(), 8) : L"           ") +
		name + type + qwtowb10(std::this_thread::get_id()._Get_underlying_id(), 6) + L" " + msg + L"\n";
}

void PublicLogger::atomicAcquire() noexcept { outStreamLock.atomicAcquire(); }
void PublicLogger::atomicRelease() noexcept { outStreamLock.atomicRelease(); }

PublicLogger::PublicLogger(const String& name): name(L" [" + name + L"] ") {
	ShowConsoleIO();
	std::wcout << L"PublicLogger created\n";
	std::wcout.imbue(std::locale("zh-CN.UTF-8"));
	mainLogger.truncate().open();
	initializerChecker.registerModule(L"Logger");
}
