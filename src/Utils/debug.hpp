#pragma once

#ifndef DEBUG

#include "base64.hpp"
#include "time.hpp"
#include "clock.hpp"

extern thread_local unsigned long int _timer_start;

#define _STRINGIFY(x) #x
#define _dbgprintf(file, line, ...) { \
    printf("[" file ":" _STRINGIFY(line) ":%s] ", __func__); \
    printf(__VA_ARGS__); \
    fflush(stdout); \
    }
#define dbgprintf(...) _dbgprintf(__FILE__, __LINE__, __VA_ARGS__)

#define _dbgprintftime(file, line, ...) { \
    printf("%+04.02fms [" file ":" _STRINGIFY(line) ":%s]\n\t", TO_MS_F(FROM_NS((get_time_ns() - _timer_start))), __func__); \
    printf(__VA_ARGS__); \
    fflush(stdout); \
    }
#define dbgprintftime(...) _dbgprintftime(__FILE__, __LINE__, __VA_ARGS__)
#define dbgtime_reset() _timer_start = get_time_ns()

#define dbgperror(x) {perror(x);}

#define dbgdumpbuffer(prefix, buffer, buflen) { \
    unsigned char* b64 = base64_encode(reinterpret_cast<const unsigned char*>(buffer), static_cast<size_t>(buflen), nullptr); \
    printf("%s%s", prefix, b64); \
    fflush(stdout); \
    free(b64);}

#include "profiler.hpp"

#else

#define dbgprintf(...)
#define dbgperror(x)
#define dbgdumpbuffer(prefix, buffer, buflen)
#define PROFILE_BLOCK

#endif
