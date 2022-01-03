
#pragma once

#include <stddef.h>
#include <type_traits>

template<size_t N, typename T = float>
struct vec
{
    T values[N];

    constexpr vec(): values{ 0 }
    {

    }

    template<typename container, typename = std::enable_if_t<(!std::is_array_v<container>)>>
    constexpr vec(const container& c)
    {
        auto iter = c.begin();
        for(int i = 0; i < N && iter != c.end(); ++i)
        {
            values[i] = *iter;
            ++iter;
        }
    }

    template<typename array, typename = std::enable_if_t<(std::is_array_v<array>)>>
    constexpr vec(const array(&c)[N])
    {
        for(int i = 0; i < N; ++i)
        {
            values[i] = c[i];
        }
    }

    constexpr int size()
    {
        return N;
    }

#define OP(fn, init, expr, ret) \
    auto fn(const T(&list)[N]) noexcept { init; for(int i = 0; i < N; ++i) { auto v = list[i]; expr; } return ret; } \
    template<typename OT> auto fn(const vec<N, OT>& other) noexcept { init; for(int i = 0; i < N; ++i) { auto v = other.values[i]; expr; } return ret; }

#define COP(fn, init, expr, ret) \
    constexpr auto fn(const T(&list)[N]) noexcept { init; for(int i = 0; i < N; ++i) { auto v = list[i]; expr; } return ret; } \
    template<typename OT> constexpr auto fn(const vec<N, OT>& other) noexcept { init; for(int i = 0; i < N; ++i) { auto v = other.values[i]; expr; } return ret; }

    OP(operator=, , values[i]=v, *this)

    COP(operator+, const T buffer[N], buffer[i]=values[i]+v, vec<N>(buffer))
    COP(operator-, const T buffer[N], buffer[i]=values[i]-v, vec<N>(buffer))
    COP(operator*, const T buffer[N], buffer[i]=values[i]*v, vec<N>(buffer))
    COP(operator/, const T buffer[N], buffer[i]=values[i]/v, vec<N>(buffer))
    COP(operator%, const T buffer[N], buffer[i]=values[i]%v, vec<N>(buffer))
    OP(operator+=, , values[i]+=v, *this)
    OP(operator-=, , values[i]-=v, *this)
    OP(operator*=, , values[i]*=v, *this)
    OP(operator/=, , values[i]/=v, *this)
    OP(operator%=, , values[i]%=v, *this)
    COP(operator==, bool acc = true, acc &= values[i]==v, acc)
    COP(operator!=, bool acc = true, acc &= values[i]!=v, acc)

    T operator[](size_t index) const noexcept
    {
        static_assert(index >= 0 && index < N);
        return values[index];
    }

    T& operator[](size_t index) noexcept
    {
        static_assert(index >= 0 && index < N);
        return values[index];
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

#undef COP
#undef OP
};
