//
// Created by EmsiaetKadosh on 25-3-6.
//

#include "gc.h"
#include "utils.h"

String $LimitedAccess::garbageStacktraceString(const std::stacktrace& stacktrace) noexcept { return stacktraceString(stacktrace); }
