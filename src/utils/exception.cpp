//
// Created by EmsiaetKadosh on 25-1-22.
//

#pragma once

#include "..\def.h"
#include "exception.h"
#include "..\utils\utils.h"
#include "..\game\Game.h"
#include "..\hbp.h"

static AtomicStorage outStreamLock;

String PublicLogger::build(const String& msg, const String& type) const {
	return L"T-" + qwtowb10(game.getTick(), 8) + name + type + qwtowb10(std::this_thread::get_id()._Get_underlying_id(), 6) + L" " + msg + L"\n";
}

void PublicLogger::atomicAcquire() noexcept { outStreamLock.atomicAcquire(); }
void PublicLogger::atomicRelease() noexcept { outStreamLock.atomicRelease(); }

PublicLogger::PublicLogger(const String& name): name(L" [" + name + L"] ") {
	ShowConsoleIO();
	std::wcout << L"PublicLogger created\n";
	std::wcout.imbue(std::locale("zh-CN.UTF-8"));
	mainLogger.truncate().open();
}
