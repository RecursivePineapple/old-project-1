
#pragma once

#include <algorithm>
#include <uuid/uuid.h>

namespace dbwrapper
{
    struct uuid_t
    {
        unsigned char value[16];

        constexpr uuid_t() : value{ 0 } { }

        uuid_t(::uuid_t const& v)
        {
            std::copy(&v[0], &v[16], value);
        }
    };
}
