
#pragma target server

#include <string>

#include <uuid/uuid.h>

#include "libpq_convert.hpp"

namespace dbwrapper
{
namespace libpq
{

#define FROM_PG_SIG(pgtype) std::optional<conversion<pgtype>::ctype> pgtype ## _from_pg(const std::string& data)
#define TO_PG_SIG(pgtype) std::string pgtype ## _to_pg(const conversion<pgtype>::ctype& data)

#define FROM_PG_TYPEDEFS(pgtype) typedef conversion<pgtype>::ctype T; typedef std::optional<T> R; typedef const std::string& P;
#define TO_PG_TYPEDEFS(pgtype) typedef conversion<pgtype>::ctype T; typedef std::string R; typedef const T& P;

#define FROM_PG_EXPR(pgtype, expr) FROM_PG_SIG(pgtype) \
    { FROM_PG_TYPEDEFS(pgtype); return expr; }

#define TO_PG_EXPR(pgtype, expr) TO_PG_SIG(pgtype) \
    { TO_PG_TYPEDEFS(pgtype); return expr; }

#define TO_PG_TO_STRING(pgtype) TO_PG_SIG(pgtype) \
    { return std::to_string(data); }

#define FROM_PG_INT(pgtype) FROM_PG_SIG(pgtype) \
    { FROM_PG_TYPEDEFS(pgtype); \
      const auto d = decimal_t::parse(data.begin(), data.end()); \
      return d ? d.value().to_int<T>() : R(); \
    }

#define FROM_PG_FLOAT(pgtype) FROM_PG_SIG(pgtype) \
    { FROM_PG_TYPEDEFS(pgtype); \
      const auto d = decimal_t::parse(data.begin(), data.end()); \
      return d ? d.value().to_float<T>() : R(); \
    }

#define FROM_PG_ARRAY(pgtype) FROM_PG_SIG(pgtype) { return array_from_pg<pgtype>(data); }
#define TO_PG_ARRAY(pgtype) TO_PG_SIG(pgtype) { return array_to_pg<pgtype>(data); }

template<basic_dtype ARRAY>
std::optional<typename conversion<ARRAY>::ctype> array_from_pg(const std::string& data)
{
    if(data.front() != '{' || data.back() != '}')
    {
        return std::optional<typename conversion<ARRAY>::ctype>();
    }

    std::vector<std::optional<typename conversion<ARRAY>::array_type::ctype>> v;
    std::string buf;
    buf.reserve(data.length());

    bool in_quote = false;
    bool escaped = false;

    const auto e = --data.end();
    for(auto iter = ++data.begin(); iter < e; ++iter)
    {
        auto c = *iter;

        if(c == '\\')
        {
            escaped = true;
        }
        else if(escaped)
        {
            buf.push_back(c);
            escaped = false;
        }
        else if(in_quote && c != '"')
        {
            buf.push_back(c);
        }
        else if(c == '"')
        {
            in_quote = !in_quote;
        }
        else if(c == ',')
        {
            v.push_back(conversion<ARRAY>::array_type::from_pg(buf));
            buf.clear();
        }
        else
        {
            buf.push_back(c);
        }
    }

    if(!buf.empty())
    {
        v.push_back(conversion<ARRAY>::array_type::from_pg(buf));
    }

    return v;
}

template<basic_dtype ARRAY>
std::string array_to_pg(const typename conversion<ARRAY>::ctype& x)
{
    std::string s;

    s = "{";

    auto iter = x.begin();
    while(iter != x.end())
    {
        s += conversion<ARRAY>::array_type::to_pg(*iter);

        if(++iter != x.end())
        {
            s += ", ";
        }
    }

    s += "}";

    return s;
}


FROM_PG_SIG(BOOL)
{
    if(data == "true" || data == "t" || data == "TRUE")
    {
        return true;
    }
    else if(data == "false" || data == "f" || data == "FALSE")
    {
        return false;
    }
    else
    {
        return std::optional<conversion<BOOL>::ctype>();
    }
}

TO_PG_EXPR(BOOL, data ? "true" : "false")

FROM_PG_EXPR(BYTEA, T(data.begin(), data.end()))
TO_PG_EXPR(BYTEA, R(data.begin(), data.end()))

FROM_PG_EXPR(CHAR, data[0])
TO_PG_TO_STRING(CHAR)

FROM_PG_EXPR(NAME, data)
TO_PG_EXPR(NAME, data)

FROM_PG_INT(INT8)
TO_PG_TO_STRING(INT8)

FROM_PG_INT(INT2)
TO_PG_TO_STRING(INT2)

FROM_PG_INT(INT4)
TO_PG_TO_STRING(INT4)

FROM_PG_EXPR(TEXT, data)
TO_PG_EXPR(TEXT, data)

FROM_PG_FLOAT(FLOAT4)
TO_PG_TO_STRING(FLOAT4)

FROM_PG_FLOAT(FLOAT8)
TO_PG_TO_STRING(FLOAT8)

FROM_PG_EXPR(MONEY, decimal_t::parse(data.begin(), data.end()))
TO_PG_EXPR(MONEY, data.to_string())

FROM_PG_EXPR(VARCHAR, data)
TO_PG_EXPR(VARCHAR, data)

FROM_PG_EXPR(NUMERIC, decimal_t::parse(data.begin(), data.end()))
TO_PG_EXPR(NUMERIC, data.to_string())

FROM_PG_EXPR(CSTRING, data)
TO_PG_EXPR(CSTRING, data)

FROM_PG_SIG(UUID)
{
    uuid_t id;
    if(uuid_parse(data.c_str(), id.value))
    {
        return id;
    }
    else
    {
        return std::optional<uuid_t>();
    }
}

TO_PG_SIG(UUID)
{
    std::string s;
    s.resize(UUID_STR_LEN);
    uuid_unparse(data.value, s.data());
    return s;
}

FROM_PG_ARRAY(BOOLARRAY)
TO_PG_ARRAY(BOOLARRAY)

FROM_PG_ARRAY(BYTEAARRAY)
TO_PG_ARRAY(BYTEAARRAY)

FROM_PG_ARRAY(CHARARRAY)
TO_PG_ARRAY(CHARARRAY)

FROM_PG_ARRAY(NAMEARRAY)
TO_PG_ARRAY(NAMEARRAY)

FROM_PG_ARRAY(INT8ARRAY)
TO_PG_ARRAY(INT8ARRAY)

FROM_PG_ARRAY(INT2ARRAY)
TO_PG_ARRAY(INT2ARRAY)

FROM_PG_ARRAY(INT4ARRAY)
TO_PG_ARRAY(INT4ARRAY)

FROM_PG_ARRAY(TEXTARRAY)
TO_PG_ARRAY(TEXTARRAY)

FROM_PG_ARRAY(FLOAT4ARRAY)
TO_PG_ARRAY(FLOAT4ARRAY)

FROM_PG_ARRAY(FLOAT8ARRAY)
TO_PG_ARRAY(FLOAT8ARRAY)

FROM_PG_ARRAY(MONEYARRAY)
TO_PG_ARRAY(MONEYARRAY)

FROM_PG_ARRAY(VARCHARARRAY)
TO_PG_ARRAY(VARCHARARRAY)

FROM_PG_ARRAY(NUMERICARRAY)
TO_PG_ARRAY(NUMERICARRAY)

FROM_PG_ARRAY(CSTRINGARRAY)
TO_PG_ARRAY(CSTRINGARRAY)

}
}