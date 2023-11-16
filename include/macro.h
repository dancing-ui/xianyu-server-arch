#ifndef MACRO_H
#define MACRO_H

#include <assert.h>

#include "log.h"

#define XIANYU_ASSERT(x) \
    if(!(x)) \
    { \
        XIANYU_LOG_ERROR(XIANYU_LOG_ROOT()) << "ASSERTION: " << #x << "\nbacktrace:\n" << xianyu::BacktraceToString(100, 2, "   "); \
        assert(x); \
    }

#define XIANYU_ASSERT2(x, w) \
    if(!(x)) \
    { \
        XIANYU_LOG_ERROR(XIANYU_LOG_ROOT()) << "ASSERTION: " << #x << '\n' << w << "\nbacktrace:\n" << xianyu::BacktraceToString(100, 2, "   "); \
        assert(x); \
    }

#endif // MACRO_H
