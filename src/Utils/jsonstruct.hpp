
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <string.h>
#include <stdexcept>
#include <variant>

#include <iostream>

#define JSMN_STATIC
#define JSMN_PARENT_LINKS
#include "jsmn.hpp"

#include "decimal.hpp"

namespace jsonstruct_utils
{
    template <auto func, std::size_t I>
    struct param_type;

    template <typename Ret, typename... Args, Ret (*func)(Args...), std::size_t I>
    struct param_type<func, I>
    {
        using type = std::tuple_element_t<I, std::tuple<Args...>>;
    };

    template <typename Ret, typename... Args, Ret (*func)(Args..., ...), std::size_t I>
    struct param_type<func, I>
    {
        using type = std::tuple_element_t<I, std::tuple<Args...>>;
    };

    template<auto func, size_t I>
    using param_type_t = typename param_type<func, I>::type;

    template<typename T, typename R>
    static const R& convert(T const& t)
    {
        return t;
    }

    template<typename T, typename R>
    static R convert(T const& t)
    {
        return static_cast<R>(t);
    }

    template<typename, typename R>
    static R convert(std::optional<R> const& o)
    {
        return o.value();
    }
}

#define _DECL_STRUCT_FIELD_X(ctype, type, name) ctype name = ctype();

#define _DECL_STRUCT_E_FIELDNO_X(ctype, type, name) fieldno_##name,

#define _PARSE_STRUCT_FIELD_X(ctype, type, name) \
    case field_numbers::fieldno_##name: { \
        std::remove_reference_t<jsonstruct_utils::param_type_t<type::parse, 3>> v; \
        if(!type::parse(__toks, __tok, __str, v)) { return false; } \
        __value.name = v; \
        break; \
    }

#define _EMIT_STRUCT_FIELD_X(_ctype, type, name) \
    s << #name ":"; type::emit(s, jsonstruct_utils::convert<ctype, type::ctype>(__value.name)); \
    if(static_cast<int>(field_numbers::fieldno_##name) < static_cast<int>(field_numbers::fn_count) - 1) s << ",";

#define _FIELDS_PUSH_X(ctype, type, name) _fields[#name] = field_numbers::fieldno_##name;

#define _FIELDS_PUSH2_X(ctype, type, name) if(key == #name) { n=field_numbers::fieldno_##name; return true; }

#define _FIELDNO_ALGO_IFS(X_FIELDS) \
    static bool fieldno(std::string const& key, field_numbers &n) { \
        (void)key;(void)n; \
        X_FIELDS(_FIELDS_PUSH2_X) \
        return false; \
    }

#define _FIELDNO_ALGO_MAP(X_FIELDS) \
    static bool fieldno(std::string const& name, field_numbers &n) { \
        static int initialized = 0; static std::map<std::string, field_numbers>  _fields; \
        if(!initialized) { \
            X_FIELDS(_FIELDS_PUSH_X) \
            initialized = 1; \
        } \
        auto e = _fields.find(name); \
        if(e != _fields.end()) { \
            n = e->second; \
            return true; \
        } else { \
            return false;\
        } \
    }

#define _DECLARE_JSON_STRUCT_BODY(name, X_FIELDS, FIELDNO_ALGO) \
        typedef name ctype; \
        enum class field_numbers: int { X_FIELDS(_DECL_STRUCT_E_FIELDNO_X) fn_count }; \
        X_FIELDS(_DECL_STRUCT_FIELD_X) \
        static bool parse(const jsmntok_t *__toks, int __tok, const char *__str, name &__value) { \
            (void)__value; \
            if(__toks[__tok].type != JSMN_OBJECT) return false;  \
            int size = __toks[__tok].size; \
            __tok++; \
            std::string k; \
            for(int i = 0; i < size; ++i) { \
                if(!jsontypes::string::parse(__toks, __tok, __str, k)) { return false; } \
                __tok = jsontypes::next_token(__toks, __tok); \
                field_numbers fn; \
                if(!fieldno(k, fn)) { __tok = jsontypes::next_token(__toks, __tok); continue; } \
                switch(fn) { \
                    X_FIELDS(_PARSE_STRUCT_FIELD_X) \
                    default: break; \
                } \
                __tok = jsontypes::next_token(__toks, __tok); \
            } \
            return true; \
        } \
        template<typename stream> \
        static stream& emit(stream &s, name const& __value) { \
            (void)__value; \
            s << "{"; \
            X_FIELDS(_EMIT_STRUCT_FIELD_X) \
            s << "}"; \
            return s; \
        } \
    private: \
        FIELDNO_ALGO(X_FIELDS) \
    public:

#define _DECLARE_JSON_STRUCT(name, X_FIELDS, FIELDNO_ALGO) \
    struct name { \
        _DECLARE_JSON_STRUCT_BODY(name, X_FIELDS, FIELDNO_ALGO) \
    };

#define DECLARE_JSON_STRUCT_BODY_SMALL(name, X_FIELDS) _DECLARE_JSON_STRUCT_BODY(name, X_FIELDS, _FIELDNO_ALGO_IFS)
#define DECLARE_JSON_STRUCT_BODY_LARGE(name, X_FIELDS) _DECLARE_JSON_STRUCT_BODY(name, X_FIELDS, _FIELDNO_ALGO_MAP)
#define DECLARE_JSON_STRUCT_BODY(name, X_FIELDS) DECLARE_JSON_STRUCT_BODY_SMALL(name, X_FIELDS)

#define DECLARE_JSON_STRUCT_SMALL(name, X_FIELDS) _DECLARE_JSON_STRUCT(name, X_FIELDS, _FIELDNO_ALGO_IFS)
#define DECLARE_JSON_STRUCT_LARGE(name, X_FIELDS) _DECLARE_JSON_STRUCT(name, X_FIELDS, _FIELDNO_ALGO_MAP)
#define DECLARE_JSON_STRUCT(name, X_FIELDS) DECLARE_JSON_STRUCT_SMALL(name, X_FIELDS)

namespace fastconvert
{
    // originally from https://tombarta.wordpress.com/2008/04/23/specializing-atoi/
    template<typename int_t = int32_t>
    static inline int_t fast_atoi(const char *str, size_t len)
    {
        int_t value = 0;
        int sign = 1;
        int exp = 1;
        if (str[0] == '-')
        {
            sign = -1;
            ++str;
            --len;
        }

        for(int i = len - 1; i >= 0; --i)
        {
            value += (str[i] - '0') * exp;
            exp *= 10;
        }

        asm("");
        return value * sign;
    }

    // originally from https://gist.github.com/oschonrock/a410d4bec6ec1ccc5a3009f0907b3d15

    // Original crack_atof version is at http://crackprogramming.blogspot.sg/2012/10/implement-atof.html
    // But it cannot convert floating point with high +/- exponent.
    // The version below by Tian Bo fixes that problem and improves performance by 10%
    // http://coliru.stacked-crooked.com/a/2e28f0d71f47ca5e
    // Oliver Schonrock: I picked this code up from
    // https://www.codeproject.com/Articles/1130262/Cplusplus-string-view-Conversion-to-Integral-Types
    // See there for benchmarking. It's blistering fast. 
    // I am sure it's not 10000% "correct", but when summing 1'000'000 parsed doubles for me in a test,
    // it obtained the exact same result as the vastly slower std::stod. Good enough for me.
    // I recfactored it slightly, changing the signature, see below.
    static inline double pow10(int n) {
        double ret = 1.0;
        double r   = 10.0;
        if (n < 0) {
            n = -n;
            r = 0.1;
        }

        while (n) {
            if (n & 1) {
            ret *= r;
            }
            r *= r;
            n >>= 1;
        }
        return ret;
    }

    // this is the same signature as from_chars (which doesn't work for float on gcc/clang)
    // ie it is a [start, end)  (not including *end). Well suited to parsing read only memorymappedfile
    static inline double fast_atof(const char* num, const char* const end) {
        if (!num || !end || end <= num) {
            return 0;
        }

        int sign         = 1;
        double int_part  = 0.0;
        double frac_part = 0.0;
        bool has_frac    = false;
        bool has_exp     = false;

        // +/- sign
        if (*num == '-') {
            ++num;
            sign = -1;
        } else if (*num == '+') {
            ++num;
        }

        while (num != end) {
            if (*num >= '0' && *num <= '9') {
                int_part = int_part * 10 + (*num - '0');
            } else if (*num == '.') {
                has_frac = true;
                ++num;
                break;
            } else if (*num == 'e') {
                has_exp = true;
                ++num;
                break;
            } else {
                return sign * int_part;
            }
            ++num;
        }

        if (has_frac) {
            double frac_exp = 0.1;

            while (num != end) {
                if (*num >= '0' && *num <= '9') {
                    frac_part += frac_exp * (*num - '0');
                    frac_exp *= 0.1;
                } else if (*num == 'e') {
                    has_exp = true;
                    ++num;
                    break;
                } else {
                    return sign * (int_part + frac_part);
                }
                ++num;
            }
        }

        // parsing exponent part
        double exp_part = 1.0;
        if (num != end && has_exp) {
            int exp_sign = 1;
            if (*num == '-') {
                exp_sign = -1;
                ++num;
            } else if (*num == '+') {
                ++num;
            }

            int e = 0;
            while (num != end && *num >= '0' && *num <= '9') {
                e = e * 10 + *num - '0';
                ++num;
            }

            exp_part = pow10(exp_sign * e);
        }

        return sign * (int_part + frac_part) * exp_part;
    }
}

namespace jsontypes
{
    template<typename T>
    struct null_t
    {
        T value;
        bool is_null;

        constexpr null_t(std::nullptr_t) : value(), is_null(true) { }
        constexpr null_t(T const& p_value): value(p_value), is_null(false) { }
        constexpr null_t(T && p_value): value(std::move(p_value)), is_null(false) { }

        constexpr operator bool() { return is_null; }

        constexpr operator T&() { return value; }

        constexpr T& operator*() { return value; }
    };
    
    struct span_t
    {
        const jsmntok_t *toks;
        int start, end; // end is inclusive
        const char *str;

        template<typename T>
        bool ParseInto(T &value)
        {
            return T::parse(toks, start, str, value);
        }

        std::string Text()
        {
            return std::string(str, static_cast<size_t>(end - start + 1));
        }
    };

    #define IMPLEMENT_PARSER(name, type) static bool name ## _parse(const jsmntok_t *toks, int tok, const char *str, type &value)
    #define IMPLEMENT_EMITTER(name, type) template<typename stream> static stream& name ## _emit(stream &s, type const&value)

    #define TOK_TEXT (std::string_view(&str[toks[tok].start], static_cast<size_t>(toks[tok].end - toks[tok].start)))

    #define TOK_CHAR (str[toks[tok].start])

    #define TOK_TYPE (toks[tok].type)

    #define TOK_ASSERT_TYPE(type) if(TOK_TYPE != type) return false

    template<typename T>
    using ctype_t = typename T::ctype;

    static inline int next_token(const jsmntok_t *toks, int tok)
    {
        auto const& t = toks[tok];
        switch(t.type)
        {
            case JSMN_STRING:
            case JSMN_PRIMITIVE:
            default: {
                return tok + 1;
            }
            case JSMN_OBJECT: {
                ++tok;
                for(int i = 0; i < t.size; ++i) {
                    tok = next_token(toks, tok);
                    tok = next_token(toks, tok);
                }
                return tok;
            }
            case JSMN_ARRAY: {
                ++tok;
                for(int i = 0; i < t.size; ++i) {
                    tok = next_token(toks, tok);
                }
                return tok;
            }
        }
    }

    IMPLEMENT_PARSER(string, std::string)
    {
        auto const& t = toks[tok];
        if(t.type != JSMN_STRING && t.type != JSMN_PRIMITIVE) return false;

        value.assign(&str[t.start], static_cast<size_t>(t.end - t.start));
        return true;
    }
    IMPLEMENT_EMITTER(string, std::string)
    {
        s << "\"" << value << "\"";
        return s;
    }

    IMPLEMENT_PARSER(boolean, bool)
    {
        auto const& t = toks[tok];
        if(t.type != JSMN_PRIMITIVE) return false;
        switch(str[t.start]) {
            case 't':{
                value = true;
                return true;
            }
            case 'f': {
                value = false;
                return true;
            }
            default: {
                return false;
            }
        }
    }
    IMPLEMENT_EMITTER(boolean, bool)
    {
        s << (value ? "true" : "false");
        return s;
    }

    IMPLEMENT_PARSER(integer, int)
    {
        auto const& t = toks[tok];
        if(t.type != JSMN_PRIMITIVE) return false;
        char c = str[t.start];
        if(c == '-' || (c >= '0' && c <= '9'))
        {
            value = fastconvert::fast_atoi(&str[t.start], static_cast<size_t>(t.end - t.start));
            return true;
        }
        else
        {
            return false;
        }
    }
    IMPLEMENT_EMITTER(integer, int)
    {
        s << value;
        return s;
    }

    IMPLEMENT_PARSER(floating, double)
    {
        auto const& t = toks[tok];
        if(t.type != JSMN_PRIMITIVE) return false;
        char c = str[t.start];
        if(c == '-' || (c >= '0' && c <= '9'))
        {
            value = fastconvert::fast_atof(&str[t.start], &str[t.end+1]);
            return true;
        }
        else
        {
            return false;
        }
    }
    IMPLEMENT_EMITTER(floating, double)
    {
        s << value;
        return s;
    }

    IMPLEMENT_PARSER(decimal, decimal_t)
    {
        auto const& t = toks[tok];
        if(t.type != JSMN_PRIMITIVE) return false;
        char c = str[t.start];
        if(c == '-' || (c >= '0' && c <= '9'))
        {
            auto d = decimal_t::parse(&str[t.start], &str[t.end+1]);
            if(d)
            {
                value = std::move(d.value());
                return true;
            }
        }
        return false;
    }
    IMPLEMENT_EMITTER(decimal, decimal_t)
    {
        s << value.to_string();
        return s;
    }
    
    template<typename T>
    static bool nullable_parse(const jsmntok_t *toks, int tok, const char *str, null_t<ctype_t<T>> &value)
    {
        auto const& t = toks[tok];
        if(t.type == JSMN_PRIMITIVE && str[t.start] == 'n')
        {
            value = null_t<T>(nullptr);
            return true;
        }
        else
        {
            ctype_t<T> v;
            if(T::parse(toks, tok, str, v))
            {
                value = null_t<T>(std::move(v));
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    template<typename T, typename stream>
    static stream& nullable_emit(stream &s, null_t<ctype_t<T>> const& value)
    {
        if(value.is_null)
        {
            s << "null";
            return s;
        }
        else
        {
            return T::emit(s, value.value);
        }
    }
    
    template<typename T>
    static bool array_parse(const jsmntok_t *toks, int tok, const char *str, std::vector<ctype_t<T>> &value)
    {
        TOK_ASSERT_TYPE(JSMN_ARRAY);

        auto const& t = toks[tok];

        value.clear();
        value.resize(static_cast<size_t>(t.size));

        tok++;

        ctype_t<T> tv;
        for(int i = 0; i < t.size; ++i)
        {
            if(!T::parse(toks, tok, str, tv))
            {
                return false;
            }

            tok = next_token(toks, tok);

            value[static_cast<size_t>(i)] = std::move(tv);
        }

        return true;
    }

    template<typename T, typename stream>
    static stream& array_emit(stream &s, std::vector<ctype_t<T>> const& value)
    {
        s << "[";

        for(size_t i = 0; i < value.size(); ++i)
        {
            switch(i)
            {
                default:
                    s << ",";
                    goto zero;
                case 0:
                zero:
                    T::emit(s, value[i]);
                    break;
            }
        }

        s << "]";

        return s;
    }
    
    template<typename K, typename V>
    static bool object_parse(const jsmntok_t *toks, int tok, const char *str, std::map<ctype_t<K>, ctype_t<V>> &value)
    {
        TOK_ASSERT_TYPE(JSMN_OBJECT);

        auto const& t = toks[tok];

        value.clear();

        tok++;

        ctype_t<K> k;
        ctype_t<V> v;
        for(int i = 0; i < t.size; ++i)
        {
            if(!K::parse(toks, tok, str, k))
            {
                return false;
            }

            tok = next_token(toks, tok);

            if(!V::parse(toks, tok, str, v))
            {
                return false;
            }

            tok = next_token(toks, tok);

            value[std::move(k)] = std::move(v);
        }

        return true;
    }

    template<typename K, typename V, typename stream>
    static stream& object_emit(stream &s, std::map<ctype_t<K>, ctype_t<V>> const& value)
    {
        s << "{";

        int i = 0;
        
        for(auto iter = value.begin(); iter != value.end(); ++i, ++iter)
        {
            switch(i)
            {
                default:
                    s << ",";
                    goto zero;
                case 0:
                zero:
                    K::emit(s, iter->first);
                    s << ":";
                    V::emit(s, iter->second);
                    break;
            }
        }

        s << "}";

        return s;
    }
    
    template<typename K, typename V>
    static bool pairs_parse(const jsmntok_t *toks, int &tok, const char *str, std::vector<std::pair<ctype_t<K>, ctype_t<V>>> &value)
    {
        TOK_ASSERT_TYPE(JSMN_OBJECT);

        auto const& t = toks[tok];

        value.clear();

        tok++;

        ctype_t<K> k;
        ctype_t<V> v;
        for(int i = 0; i < t.size; ++i)
        {
            if(!K::parse(toks, tok, str, k))
            {
                return false;
            }

            tok = next_token(toks, tok);

            if(!V::parse(toks, tok, str, v))
            {
                return false;
            }

            tok = next_token(toks, tok);

            value.push_back(std::make_pair(std::move(k, std::move(v))));
        }

        return true;
    }

    template<typename K, typename V, typename stream>
    static stream& pairs_emit(stream &s, std::map<ctype_t<K>, ctype_t<V>> const& value)
    {
        s << "{";

        int i = 0;
        
        for(auto iter = value.begin(); iter != value.end(); ++i, ++iter)
        {
            switch(i)
            {
                default:
                    s << ",";
                    goto zero;
                case 0:
                zero:
                    K::emit(s, iter->first);
                    s << ":";
                    V::emit(s, iter->second);
                    break;
            }
        }

        s << "}";

        return s;
    }
    
    struct string
    {
        typedef std::string ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return string_parse(toks, tok, str, value);
        }

        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return string_emit(s, value);
        }
    };

    struct boolean
    {
        typedef bool ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return boolean_parse(toks, tok, str, value);
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return boolean_emit(s, value);
        }
    };

    struct integer
    {
        typedef int ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return integer_parse(toks, tok, str, value);
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return integer_emit(s, value);
        }
    };

    struct floating
    {
        typedef double ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return floating_parse(toks, tok, str, value);
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return floating_emit(s, value);
        }
    };

    template<typename T>
    struct nullable
    {
        typedef null_t<ctype_t<T>> ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return nullable_parse<T>(toks, tok, str, value);
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return nullable_emit<T, stream>(s, value);
        }
    };

    template<typename T>
    struct array
    {
        typedef std::vector<ctype_t<T>> ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return array_parse<T>(toks, tok, str, value);
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return array_emit<T, stream>(s, value);
        }
    };

    template<typename K, typename V>
    struct object
    {
        typedef std::map<ctype_t<K>, ctype_t<V>> ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return object_parse<K, V>(toks, tok, str, value);
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return object_emit<K, V, stream>(s, value);
        }
    };

    template<typename K, typename V>
    struct pairs
    {
        typedef std::vector<std::pair<ctype_t<K>, ctype_t<V>>> ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return pairs_parse<K, V>(toks, tok, str, value);
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return pairs_emit<K, V, stream>(s, value);
        }
    };

    struct span
    {
        typedef span_t ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            value = span_t();
            value.toks = toks;
            value.start = tok;
            value.end = next_token(toks, tok);
            value.str = str;
            return true;
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            (void)value;
            throw std::runtime_error("cannot emit span");
            return s;
        }

        static void foo() { }
    };

    template<typename... T>
    struct variant
    {
    private:
        template<typename _T>
        struct _has_parse
        {
        private:
            template<typename C> static char  check(decltype(&C::parse));
            template<typename C> static short check(...);
        public:
            enum { value = sizeof(check<_T>(0)) == sizeof(char) };
        };
        template<typename _T>
        struct _has_emit
        {
        private:
            template<typename C> static char  check(decltype(&C::emit));
            template<typename C> static short check(...);
        public:
            enum { value = sizeof(check<_T>(0)) == sizeof(char) };
        };
    public:

        typedef std::variant<ctype_t<T>...> ctype;

        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            return (... || ([&]() {
                if constexpr(_has_parse<T>::value)
                {
                    std::decay_t<jsonstruct_utils::param_type_t<T::parse, 3>> parser_value;
                    if(T::parse(toks, tok, str, parser_value))
                    {
                        value = std::move(parser_value);
                        return true;
                    }
                }
                return false;
            })());
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            std::visit([&s](auto&& arg) {

                (... || ([&s](auto const& arg) {
                    using TParam = std::decay_t<decltype(arg)>;
                    using TEmitterParam = std::decay_t<jsonstruct_utils::param_type_t<T::template emit<stream>, 1>>;
                    if constexpr(_has_emit<T>::value && std::is_same_v<TParam, TEmitterParam>)
                    {
                        T::emit(s, arg);
                        return true;
                    }
                    return false;
                })(arg));

            }, value);
            return s;
        }
    };

}
