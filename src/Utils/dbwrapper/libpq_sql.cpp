
#pragma target server

#include "libpq_sql.hpp"

#include "util.hpp"

namespace dbwrapper
{
namespace libpq
{

std::string to_sql(const entity_name& name)
{
    return name.m_schema + "." + name.m_name;
}

std::string to_sql_value(const column &col)
{
    std::stringstream s;
    if(col.m_alias)
    {
        s << col.m_table << "." << col.m_name << " AS " << col.m_alias.value();
    }
    else
    {
        s << col.m_table << "." << col.m_name;
    }
    return s.str();
}

std::string to_sql_value(const dtype& d)
{
    return d.m_type;
}

std::string to_sql_value(const param& p)
{
    return "$" + std::to_string(p.m_id) + p.m_dtype;
}

std::string to_sql_value_ref(const column& col)
{
    std::stringstream s;
    if(col.m_alias)
    {
        s << col.m_alias.value();
    }
    else
    {
        s << col.m_table << "." << col.m_name;
    }
    return s.str();
}

std::string to_sql_value(const literal &l)
{
    return l.m_sql;
}

std::string to_sql_table_from(const table &t)
{
    if(t.m_alias.empty())
    {
        return to_sql(t.m_name);
    }
    else
    {
        return to_sql(t.m_name) + " AS " + t.m_alias;
    }
}

std::string to_sql_table_ref(const table &t)
{
    if(t.m_alias.empty())
    {
        return to_sql(t.m_name);
    }
    else
    {
        return t.m_alias;
    }
}

std::string to_sql(const query& q)
{
    std::stringstream s;

    s << "SELECT ";

    stream_join_map(s, ", ", q.m_cols, [](const column& c) {
        return to_sql_value(c);
    });

    s << " " << "FROM " << q.m_table << " ";

    for(const auto& join : q.m_joins)
    {
        s << to_sql(join) << " ";
    }

    if(q.m_constraint)
    {
        s << "WHERE " << q.m_constraint.value() << " ";
    }

    if(q.m_group_by)
    {
        s << "GROUP BY " << q.m_group_by.value() << " ";
    }

    if(q.m_having)
    {
        s << "HAVING " << q.m_having.value() << " ";
    }

    if(q.m_order)
    {
        s << "ORDER BY " << q.m_order.value() << " ";
    }

    if(q.m_limit)
    {
        s << "LIMIT " << q.m_limit.value() << " ";
    }
    else if(q.m_offset)
    {
        s << "LIMIT -1 ";
    }

    if(q.m_offset)
    {
        s << "OFFSET " << q.m_offset.value() << " ";
    }

    return s.str();
}

std::string to_sql_table_from(const query& q)
{
    std::stringstream s;

    s << "(" << to_sql(q) << ") ";

    if(q.m_alias)
    {
        s << "AS " << q.m_alias.value();
    }

    return s.str();
}

std::string to_sql_table_ref(const query& q)
{
    if(q.m_alias)
    {
        return q.m_alias.value();
    }
    else
    {
        std::stringstream s;

        s << "(" << to_sql(q) << ") ";

        return s.str();
    }
}

std::string to_sql(const join& j)
{
    std::stringstream s;

    s << j.m_join_type << " " << j.m_table << " ON " << j.m_condition;

    return s.str();
}

dtype to_dtype(const std::string& s)
{
    return dtype(s);
}

dtype to_dtype(basic_dtype d)
{
    switch(d)
    {
    #define X(id) case id: return dtype(#id);
    BASIC_DTYPES
    #undef X
    default:
        throw std::runtime_error("invalid dtype: " + std::to_string(static_cast<int>(d)));
    }
}

}
}
