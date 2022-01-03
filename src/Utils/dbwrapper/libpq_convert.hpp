#pragma once

#include <vector>
#include <string>
#include <optional>
#include <uuid/uuid.h>

#include "libpq_sql.hpp"

#include "Utils/decimal.hpp"
#include "util.hpp"
#include "uuid.hpp"

namespace dbwrapper
{
namespace libpq
{

template<basic_dtype>
struct conversion
{
    template<typename T>
    static std::optional<T> from_pg(const std::string& data);

    template<typename T>
    static std::string to_pg(const T& t);

    static dtype type();
    
    using array_type = void; 
};

#define DATATYPE_INFO_LIST \
    X(BOOL, bool) \
    X(BYTEA, std::vector<char>) \
    X(CHAR, char) \
    X(NAME, std::string) \
    X(INT8, int64_t) \
    X(INT2, int16_t) \
    X(INT4, int32_t) \
    X(TEXT, std::string) \
    X(FLOAT4, float) \
    X(FLOAT8, double) \
    X(MONEY, decimal_t) \
    X(VARCHAR, std::string) \
    X(NUMERIC, decimal_t) \
    X(CSTRING, std::string) \
    X(UUID, uuid_t) \
    X2(BOOLARRAY, BOOL) \
    X2(BYTEAARRAY, BYTEA) \
    X2(CHARARRAY, CHAR) \
    X2(NAMEARRAY, NAME) \
    X2(INT8ARRAY, INT8) \
    X2(INT2ARRAY, INT2) \
    X2(INT4ARRAY, INT4) \
    X2(TEXTARRAY, TEXT) \
    X2(FLOAT4ARRAY, FLOAT4) \
    X2(FLOAT8ARRAY, FLOAT8) \
    X2(MONEYARRAY, MONEY) \
    X2(VARCHARARRAY, VARCHAR) \
    X2(NUMERICARRAY, NUMERIC) \
    X2(CSTRINGARRAY, CSTRING)

#define _X(pgtype, _ctype, atype) \
    std::optional<_ctype> pgtype ## _from_pg(const std::string&); \
    std::string pgtype ## _to_pg(const _ctype&); \
    template<> struct conversion<pgtype> \
    { \
        static std::optional<_ctype> from_pg(const std::string& data) \
            { return data.length() > 0 ? pgtype ## _from_pg(data) : std::optional<_ctype>(); } \
        static std::string to_pg(std::optional<_ctype> t) \
            { return t ? pgtype ## _to_pg(t.value()) : ""; } \
        static dtype type() { return to_dtype(pgtype); } \
        template<typename T> constexpr static bool is() { return std::is_convertible<_ctype, T>(); } \
        typedef _ctype ctype; \
        using array_type = atype; \
    };

#define X(pgtype, _ctype) _X(pgtype, _ctype, void)

#define X2(pgtype, atype) _X(pgtype, std::vector<std::optional<conversion<atype>::ctype>>, conversion<atype>)

DATATYPE_INFO_LIST

#undef X
#undef X2

}
}
