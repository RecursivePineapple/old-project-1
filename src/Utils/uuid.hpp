
#pragma once

#include <uuid/uuid.h>

#include <utility>
#include <string>
#include <array>

#include "Utils/dbwrapper/uuid.hpp"

struct uuid
{
    uuid_t value;

    uuid()
    {
        clear();
    }

    uuid(uuid_t const& v)
    {
        uuid_copy(value, v);
    }

    uuid(std::array<unsigned char, 16> const& array)
    {
        uuid_copy(value, array.data());
    }

    uuid(dbwrapper::uuid_t const& other)
    {
        uuid_copy(value, other.value);
    }

    void clear() { uuid_clear(value); }

    int operator<(uuid const& other) const { return uuid_compare(value, other.value) < 0; }
    int operator==(uuid const& other) const { return uuid_compare(value, other.value) == 0; }
    int operator>(uuid const& other) const { return uuid_compare(value, other.value) > 0; }

    operator dbwrapper::uuid_t() const
    {
        return dbwrapper::uuid_t(value);
    }

    static uuid generate()
    {
        uuid u;
        uuid_generate(u.value);
        return u;
    }
    static uuid generate_random()
    {
        uuid u;
        uuid_generate_random(u.value);
        return u;
    }
    static uuid generate_time()
    {
        uuid u;
        uuid_generate_time(u.value);
        return u;
    }
    static uuid generate_time_safe()
    {
        uuid u;
        uuid_generate_time_safe(u.value);
        return u;
    }
    template<typename string_like>
    static uuid generate_md5(uuid const& ns, string_like const& name)
    {
        uuid u;
        uuid_generate_md5(u.value, ns.value, name.c_str(), name.size());
        return u;
    }
    template<typename string_like>
    static uuid generate_sha1(uuid const& ns, string_like const& name)
    {
        uuid u;
        uuid_generate_sha1(u.value, ns.value, name.c_str(), name.size());
        return u;
    }

    static bool parse(std::string const& text, uuid& u)
    {
        return uuid_parse(text.c_str(), u.value) == 0 ? true : false;
    }

    std::string to_string() const
    {
        std::string s;
        s.resize(UUID_STR_LEN-1);
        uuid_unparse(value, s.data());
        return s;
    }

    struct timeval time() const
    {
        struct timeval tv;
        uuid_time(value, &tv);
        return tv;
    }

    int type() const { return uuid_type(value); }
    int variant() const { return uuid_variant(value); }
};

namespace std
{
    template<>
    struct hash<uuid>
    {
        size_t operator()(uuid const& id) const noexcept
        {
            size_t h = 0;

            static_assert(sizeof(id.value) % sizeof(int) == 0);
            
            for(size_t i = 0; i < sizeof(id.value) / sizeof(int); i++)
            {
                const unsigned int * ptr = reinterpret_cast<const unsigned int*>(id.value);

                h = (h + (324723947ul + ptr[i])) ^93485734985ul;
            }
            
            return h;
        }
    };
}
