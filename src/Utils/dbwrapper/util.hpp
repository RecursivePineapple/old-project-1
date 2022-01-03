#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>

template <class F, class T, class = T>
struct is_static_castable : std::false_type {};

template <class F, class T>
struct is_static_castable<F, T, decltype(static_cast<T>(std::declval<F>()))> : std::true_type {};

template<typename T, typename R, typename = R>
struct conditional_cast
{
    conditional_cast(T) { }

    operator R()
    {
        throw std::runtime_error(
            std::string("cannot static_cast ") +
            typeid(T).name() +
            " to " +
            typeid(R).name()
        );
        R *r = nullptr;
        return *r;
    }
};

template<typename T, typename R>
struct conditional_cast<T, R, decltype(static_cast<R>(std::declval<T>()))>
{
    conditional_cast(T t): data(static_cast<R>(t)) { }

    operator R() { return data; }

    R data;
};

template<class container, class iter, typename func>
container iter_map(iter first, iter last, const func &fn)
{
    container x;

    for(; first != last; ++first)
    {
        x.insert(x.end(), fn(*first));
    }

    return x;
}

template<class container, class container2, typename func>
container iter_map(const container2 &x, const func &fn)
{
    return iter_map(x.begin(), x.end(), fn);
}

template<class stream, class iter, typename delim>
void stream_join(stream &s, delim d, iter first, iter last)
{
    if(first == last)
    {
        return;
    }

    s << *first++;
    for(; first != last; ++first)
    {
        s << d << *first;
    }
}

template<class stream, class container, typename delim>
void stream_join(stream &s, delim d, const container &c)
{
    stream_join(s, d, c.begin(), c.end());
}

template<class stream, class iter, typename delim, typename func>
void stream_join_map(stream &s, delim d, iter first, iter last, const func &fn)
{
    if(first == last)
    {
        return;
    }

    s << fn(*first++);
    for(; first != last; ++first)
    {
        s << d << fn(*first);
    }
}

template<class stream, class container, typename delim, typename func>
void stream_join_map(stream &s, delim d, const container &c, const func &fn)
{
    stream_join_map(s, d, c.begin(), c.end(), fn);
}

static void replace_all(std::string& str, const std::string& from, const std::string& to);

static void replace_all(std::string& str, const std::string& from, const std::string& to)
{
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

static std::string str_trim(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());

    return s;
}
