#pragma once

#include <string>
#include <sstream>
#include <optional>
#include <map>
#include <vector>

#ifndef RAISE_ERROR
#define RAISE_ERROR(msg) throw std::runtime_error(msg)
#endif

namespace dbwrapper
{
namespace libpq
{

struct entity_name;
struct literal;
struct database;
struct param;
struct dtype;
struct function;
struct stored_procedure;
struct transaction;
struct value_like;
struct column_like;
struct table_like;
struct table;
struct column;
struct query;
struct join;

#define BASIC_DTYPES \
    X(BOOL) \
    X(BYTEA) \
    X(CHAR) \
    X(NAME) \
    X(INT8) \
    X(INT2) \
    X(INT4) \
    X(TEXT) \
    X(FLOAT4) \
    X(FLOAT8) \
    X(MONEY) \
    X(VARCHAR) \
    X(NUMERIC) \
    X(CSTRING) \
    X(UUID) \
    X(BOOLARRAY) \
    X(BYTEAARRAY) \
    X(CHARARRAY) \
    X(NAMEARRAY) \
    X(INT8ARRAY) \
    X(INT2ARRAY) \
    X(INT4ARRAY) \
    X(TEXTARRAY) \
    X(FLOAT4ARRAY) \
    X(FLOAT8ARRAY) \
    X(MONEYARRAY) \
    X(VARCHARARRAY) \
    X(NUMERICARRAY) \
    X(CSTRINGARRAY)

enum basic_dtype : int32_t
{
#define X(id) id,
BASIC_DTYPES
#undef X
};

constexpr const char* lookup_dtype_name(basic_dtype dtype)
{
#define X(name) case name : return #name;

    switch(dtype)
    {
        BASIC_DTYPES
        default: throw std::runtime_error(std::string("illegal dtype " + std::to_string(static_cast<int>(dtype))));
    }

#undef X
}

// to_sql = can be any sql (context dependent)
// to_sql_value = must be a value
// to_sql_table = must be select-able

template<typename T>
static typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type
to_sql_value(T x)
{
    return std::to_string(x);
}

static std::string to_sql_value(std::nullptr_t)
{
    return "NULL";
}

static std::string to_sql_value(bool b)
{
    return b ? "TRUE" : "FALSE";
}

std::string to_sql_value(const column& col);
std::string to_sql_value(const literal& l);
std::string to_sql_value(const dtype& d);
std::string to_sql_value(const param& p);

std::string to_sql_value_ref(const column& col);

std::string to_sql(const entity_name& name);
std::string to_sql(const query& q);
std::string to_sql(const join& j);

std::string to_sql_table_from(const table& t);
std::string to_sql_table_from(const query& q);

std::string to_sql_table_ref(const table& t);
std::string to_sql_table_ref(const query& q);

dtype to_dtype(const std::string& s);
dtype to_dtype(basic_dtype d);

struct literal
{
    literal(const std::string &s): m_sql(s) { }

    std::string m_sql;
};

struct entity_name
{
    entity_name(const std::string &name): m_schema("public"), m_name(name) {}

    entity_name(const std::string &schema, const std::string &name): m_schema(schema), m_name(name) {}

    std::string m_schema;
    std::string m_name;
};

template<typename A>
struct sql_operators
{
#define OP(fn, sql) \
    template<typename B> \
    literal fn(const B& b) \
    { \
        std::stringstream s; \
        s << "(" << to_sql_value(*static_cast<A*>(this)) << " " sql " " << to_sql_value(b) << ")"; \
        return s.str(); \
    }
#define OP2(op, sql) OP(operator op, sql)

    OP2(==, "=" )
    OP2(!=, "<>")
    OP2(<,  "<" )
    OP2(>,  ">" )
    OP2(<=, "<=")
    OP2(>=, ">=")

    OP2(+, "+")
    OP2(-, "-")
    OP2(*, "*")
    OP2(/, "/")
    OP2(%, "%")
    OP2(^, "^")

    OP2(&&, "AND")
    OP2(||, "OR")

    OP(str_concat, "||")
    OP(is_in, "IN")
    OP(is_not_in, "NOT IN")
    OP(is_like, "~~")
    OP(is_not_like, "~~~")
    
    literal operator!()
    {
        std::stringstream s;
        s << "(NOT " << to_sql_value(*static_cast<A*>(this)) << ")";
        return s.str();
    }

    template<typename B>
    literal expr(const std::string &op, const B& b)
    {
        std::stringstream s;
        s << "(" << to_sql_value(*static_cast<A*>(this)) << " " << op << " " << to_sql_value(b) << ")";
        return s.str();
    }

#undef OP
#undef OP2
};

struct dtype
{
    dtype(const std::string &type) : m_type(type) { }

    dtype(const entity_name &type) : m_type(to_sql(type)) { }

    template<typename sql_value>
    auto cast(const sql_value& value)
    {
        return value.expr("::", *this);
    }

    std::string m_type;
};

struct param
{
    param(int id): m_id(id), m_dtype("") { }

    template<typename dtype_t>
    param(int id, const dtype_t& dtype): m_id(id), m_dtype(to_sql(dtype)) { }

    int m_id;
    std::string m_dtype;
};

struct column : public sql_operators<column>
{
    template<typename table_t>
    column(const table_t &tbl, const std::string &name):
        m_table(to_sql_table_ref(tbl)), m_name(name) { }

    column& as(const std::string &alias)
    {
        m_alias = alias;
        return *this;
    }

    std::string m_table;
    std::string m_name;
    std::optional<std::string> m_alias;
};

struct join
{
    template<typename table_t>
    join(query &q, const table_t &tbl, const std::string &join_type):
        m_query(q), m_table(to_sql_table_from(tbl)), m_join_type(join_type) { }

    template<typename condition_t>
    query& on(const condition_t &condition)
    {
        m_condition = to_sql_value(condition);
        return m_query;
    }

    query &m_query;
    std::string m_table;
    std::string m_join_type;
    std::string m_condition;
};

typedef enum
{
    NONE = 0,
    ASC = 1,
    DESC = 2
} OrderDirection;

struct query
{
    template<typename table_t>
    query(const table_t &tbl, const std::vector<column> &cols) :
        m_table(to_sql_table_from(tbl)), m_cols(cols)
    { }

    template<typename sql_value_t>
    query& where(const sql_value_t &constraint)
    {
        m_constraint = to_sql_value(constraint);
        return *this;
    }

    template<typename sql_value_t>
    query& add_group_by(const sql_value_t &grouping)
    {
        if(m_group_by)
        {
            m_group_by = m_group_by.value() + "," + to_sql_value(grouping);
        }
        else
        {
            m_group_by = to_sql_value(grouping);
        }
        return *this;
    }

    template<typename sql_value_t>
    query& having(const sql_value_t &constraint)
    {
        m_having = to_sql_value(constraint);
        return *this;
    }

    template<typename sql_value_t>
    query& add_order_by(const sql_value_t &order, OrderDirection dir = OrderDirection::NONE)
    {
        if(m_order)
        {
            m_order = m_order.value() + "," + to_sql_value(order).value;
        }
        else
        {
            m_order = to_sql_value(order);
        }

        switch(dir)
        {
            case OrderDirection::ASC:
                m_order = m_order.value() + " ASC";
                break;
            case OrderDirection::DESC:
                m_order = m_order.value() + " DESC";
                break;
            default:
                break;
        }

        return *this;
    }

    template<typename sql_value_t>
    query& limit(const sql_value_t &l)
    {
        m_limit = to_sql_value(l);
        return *this;
    }

    template<typename sql_value_t>
    query& offset(const sql_value_t &o)
    {
        m_offset = to_sql_value(o);
        return *this;
    }

    query& as(const std::string &alias)
    {
        m_alias = alias;
        return *this;
    }

    template<typename table_t>
    join& inner_join(const table_t &tbl)
    {
        return m_joins.emplace_back(*this, tbl, "INNER JOIN");
    }

    template<typename table_t>
    join& equi_join(const table_t &tbl)
    {
        return m_joins.emplace_back(*this, tbl, "JOIN");
    }

    template<typename table_t>
    join& natural_join(const table_t &tbl)
    {
        return m_joins.emplace_back(*this, tbl, "NATURAL JOIN");
    }

    template<typename table_t>
    join& left_outer_join(const table_t &tbl)
    {
        return m_joins.emplace_back(*this, tbl, "LEFT OUTER JOIN");
    }

    template<typename table_t>
    join& right_outer_join(const table_t &tbl)
    {
        return m_joins.emplace_back(*this, tbl, "RIGHT OUTER JOIN");
    }

    template<typename table_t>
    join& full_outer_join(const table_t &tbl)
    {
        return m_joins.emplace_back(*this, tbl, "FULL OUTER JOIN");
    }

    column col(const std::string &colname)
    {
        for(const auto& col : m_cols)
        {
            if(to_sql_value_ref(col) == colname)
            {
                return column(*this, colname);
            }
        }

        RAISE_ERROR("error: could not find column '" + colname + "' in query");
    }

    query select(std::initializer_list<struct column> cols)
    {
        return query(*this, std::vector<struct column>(cols));
    }

    std::string m_table;
    std::vector<column> m_cols;
    std::vector<join> m_joins;

    std::optional<std::string> m_constraint;
    std::optional<std::string> m_group_by;
    std::optional<std::string> m_having;
    std::optional<std::string> m_order;
    std::optional<std::string> m_limit;
    std::optional<std::string> m_offset;
    std::optional<std::string> m_alias;
};

struct table
{
public:

    table(database *db, const entity_name &name) : m_db(db), m_name(name), m_alias("") { }

    table(database *db, const std::string &name) : m_db(db), m_name(name), m_alias("") { }

    table& as(const std::string &alias)
    {
        m_alias = alias;
        return *this;
    }

    column col(const std::string &colname)
    {
        return column(*this, colname);
    }

    query select(std::initializer_list<struct column> cols)
    {
        return query(*this, std::vector<struct column>(cols));
    }

    const std::map<std::string, struct column>& get_columns();

    database *m_db;
    entity_name m_name;
    std::string m_alias;

    std::optional<std::map<std::string, struct column>> m_cols;
};

}
}
