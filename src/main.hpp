#pragma once

#include <Windows.h>
#include "src\utils\time.hpp"
#include "src\utils\logger.hpp"
#include "src\game\game.hpp"
#include "using.hpp"

inline const String ApplicationName = L"High Blood Pressure";

#define DECL(className, varName) inline className& varName = Details::get##className();
DECL(TimeManager, timer);
DECL(PublicLogger, Logger);
DECL(Game, game);
#undef DECL
