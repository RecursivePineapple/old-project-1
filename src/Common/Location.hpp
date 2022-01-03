
#pragma once

#include <type_traits>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>

namespace gamestate
{
    struct Location
    {
        static constexpr int N = 3;
        typedef int64_t T;
        struct comp_t {
            T x, y, z;
        };

        union {
            comp_t comp;
            T values[N];
        };

        constexpr Location(): comp{ 0, 0, 0 } { }

        constexpr Location(T _x, T _y, T _z): comp{ _x, _y, _z } { }

        constexpr Location(Location const& other) = default;
        constexpr Location(Location && other) = default;

        template<typename container, typename = std::enable_if_t<(!std::is_array_v<container>)>>
        constexpr Location(const container& c)
        {
            auto iter = c.begin();
            for(int i = 0; i < N && iter != c.end(); ++i)
            {
                values[i] = static_cast<T>(*iter);
                ++iter;
            }
        }

        template<typename array, typename = std::enable_if_t<(std::is_array_v<array>)>>
        constexpr Location(const array(&c)[N])
        {
            for(int i = 0; i < N; ++i)
            {
                values[i] = static_cast<T>(c[i]);
            }
        }

        constexpr int size()
        {
            return N;
        }

    #define OP(fn, init, expr, ret) \
        auto fn(const T(&list)[N]) noexcept { init; for(int i = 0; i < N; ++i) { auto v = list[i]; expr; } return ret; } \
        auto fn(Location const& other) noexcept { init; for(int i = 0; i < N; ++i) { auto v = other.values[i]; expr; } return ret; }

    #define COP(fn, init, expr, ret) \
        constexpr auto fn(const T(&list)[N]) noexcept { init; for(int i = 0; i < N; ++i) { auto v = list[i]; expr; } return ret; } \
        constexpr auto fn(Location const& other) noexcept { init; for(int i = 0; i < N; ++i) { auto v = other.values[i]; expr; } return ret; }

        OP(operator=, , values[i]=v, *this)

        COP(operator+, Location out, out.values[i]=values[i]+v, out)
        COP(operator-, Location out, out.values[i]=values[i]-v, out)
        COP(operator*, Location out, out.values[i]=values[i]*v, out)
        COP(operator/, Location out, out.values[i]=values[i]/v, out)
        COP(operator%, Location out, out.values[i]=values[i]%v, out)
        OP(operator+=, , values[i]+=v, *this)
        OP(operator-=, , values[i]-=v, *this)
        OP(operator*=, , values[i]*=v, *this)
        OP(operator/=, , values[i]/=v, *this)
        OP(operator%=, , values[i]%=v, *this)

        T operator[](const size_t index) const
        {
            assert(index >= N);
            
            return values[index];
        }

        T& operator[](size_t index)
        {
            assert(index >= N);
            
            return values[index];
        }

        constexpr bool operator<(Location const& other)
        {
            if(comp.x < other.comp.x) return true;
            if(comp.x > other.comp.x) return false;
            if(comp.y < other.comp.y) return true;
            if(comp.y > other.comp.y) return false;
            if(comp.z < other.comp.z) return true;
            if(comp.z > other.comp.z) return false;

            return false;
        }

        constexpr bool operator>(Location const& other)
        {
            if(comp.x > other.comp.x) return true;
            if(comp.x < other.comp.x) return false;
            if(comp.y > other.comp.y) return true;
            if(comp.y < other.comp.y) return false;
            if(comp.z > other.comp.z) return true;
            if(comp.z < other.comp.z) return false;

            return false;
        }

        constexpr bool operator==(Location const& other)
        {
            return comp.x == other.comp.x && comp.y == other.comp.y && comp.z == other.comp.z;
        }

        constexpr T len2() noexcept
        {
            T acc = 0;
            for(size_t i = 0; i < N; ++i)
            {
                T v = values[i];
                acc += v * v;
            }

            return acc;
        }

        constexpr T len() noexcept
        {
            return sqrt(len2());
        }

        constexpr T* begin()
        {
            return &values[0];
        }
        
        constexpr T* end()
        {
            return (&values[N])+1;
        }

    #undef COP
    #undef OP
    };

}