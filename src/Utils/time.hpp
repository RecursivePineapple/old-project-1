#pragma once

#include <stdint.h>

#define FROM_SEC(seconds) (seconds * 1e9)
#define FROM_MS(milliseconds) (milliseconds * 1e6)
#define FROM_MCS(microseconds) (microseconds * 1e3)
#define FROM_NS(nanoseconds) (nanoseconds)
#define FROM_TIMESPEC(ts) (ts.tv_sec * 1e9 + ts.tv_nsec)

#define TO_SEC_F(x) static_cast<float>(x / 1e9)
#define TO_MS_F(x) static_cast<float>(x / 1e6)
#define TO_MCS_F(x) static_cast<float>(x / 1e3)
#define TO_NS_F(x) static_cast<float>(x)

#define TO_SEC_I32(x) static_cast<int32_t>(x / 1e9)
#define TO_MS_I32(x) static_cast<int32_t>(x / 1e6)
#define TO_MCS_I32(x) static_cast<int32_t>(x / 1e3)
#define TO_NS_I32(x) static_cast<int32_t>(x)

#define TO_SEC_UI32(x) static_cast<uint32_t>(x / 1e9)
#define TO_MS_UI32(x) static_cast<uint32_t>(x / 1e6)
#define TO_MCS_UI32(x) static_cast<uint32_t>(x / 1e3)
#define TO_NS_UI32(x) static_cast<uint32_t>(x)

#define TO_SEC_UI64(x) static_cast<uint64_t>(x / 1e9)
#define TO_MS_UI64(x) static_cast<uint64_t>(x / 1e6)
#define TO_MCS_UI64(x) static_cast<uint64_t>(x / 1e3)
#define TO_NS_UI64(x) static_cast<uint64_t>(x)
