#pragma once

#include <time.h>

#include "time.hpp"

static inline uint64_t get_time_ns() {
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);

    return TO_NS_UI64(FROM_TIMESPEC(time));
}
