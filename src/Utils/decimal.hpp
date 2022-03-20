#pragma once

#include <vector>
#include <string>
#include <optional>
#include <stdint.h>

struct decimal_t
{
    bool sign = true;
    std::vector<uint8_t> integral, frac;

    template<typename int_t = int32_t>
    typename std::enable_if<std::is_integral<int_t>::value, int_t>::type to_int() const
    {
        int_t x = 0;
        int_t exp = 1;

        for(auto digit = --integral.end(); digit != --integral.begin(); --digit)
        {
            x += (*digit) * exp;
            exp *= 10;
        }

        return x * (sign ? 1 : -1);
    }

    template<typename float_t>
    typename std::enable_if<std::is_floating_point<float_t>::value, float_t>::type to_float() const
    {
        float_t x = 0;
        float_t exp = 1, denom = 10;

        for(auto digit = --integral.end(); digit != --integral.begin(); --digit)
        {
            x += (*digit) * exp;
            exp *= 10;
        }

        for(auto digit = --frac.end(); digit != --frac.begin(); --digit)
        {
            x += (*digit) / denom;
            denom *= 10;
        }

        return x * (sign ? 1 : -1);
    }

    std::string to_string() const
    {
        std::string s;

        for(const auto digit : integral)
        {
            s += static_cast<char>(digit + '0');
        }

        if(!frac.empty())
        {
            s += '.';
            for(const auto digit : frac)
            {
                s += static_cast<char>(digit + '0');
            }
        }

        return  (sign ? "" : "-") + s;
    }

    template<typename iter>
    static std::optional<decimal_t> parse(iter begin, iter end)
    {
        decimal_t dec;

        bool decimal_seen = false;

        if(*begin == '-')
        {
            dec.sign = false;
            ++begin;
        }

        while(begin != end)
        {
            const auto c = *begin;

            switch(c)
            {
                case '.':
                    decimal_seen = true;
                    break;
                case ',':
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if(decimal_seen)
                    {
                        dec.frac.push_back(static_cast<uint8_t>(c - '0'));
                    }
                    else
                    {
                        dec.integral.push_back(static_cast<uint8_t>(c - '0'));
                    }
                    break;
                default:
                    return std::optional<decimal_t>();
            }

            ++begin;
        }

        return dec;
    }
};
