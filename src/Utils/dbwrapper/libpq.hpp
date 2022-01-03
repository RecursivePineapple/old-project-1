#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <typeinfo>
#include <spdlog/spdlog.h>

#include <postgresql/libpq-fe.h>
#include <postgresql/14/server/catalog/pg_type_d.h>

#include "libpq_sql.hpp"
#include "libpq_convert.hpp"

#include "util.hpp"

// #include "Utils/debug.hpp"
#include "Utils/time.hpp"
#include "Utils/clock.hpp"

namespace dbwrapper
{
namespace libpq
{

struct host_info;
struct connection_info;
struct database;
struct result;
struct prepared_query;

std::string url_encode(const std::string &value);

struct host_info
{
public:
    host_info() {}

    host_info(const std::string &_hostname, uint16_t _port = 0):
        m_hostname(_hostname),
        m_port(_port)
    {}

    std::string to_string() const
    {
        if(m_port == 0)
        {
            return m_hostname;
        }
        else
        {
            return m_hostname + ":" + std::to_string(static_cast<int>(m_port));
        }
    }

    std::string m_hostname;
    uint16_t m_port;
};

struct connection_info
{
public:
    connection_info() {}

    connection_info(
        const std::string &user,
        const std::string &password,
        const std::string &hostname = "localhost",
        const std::string &database = "postgres"):
        m_user(user), m_password(password), m_database(database), m_hosts{host_info(hostname)}
    { }

    connection_info(const std::map<std::string, std::string> &params): m_params(params)
    { }

    std::string to_string() const;

    std::string m_user;
    std::string m_password;
    std::string m_database;
    std::vector<host_info> m_hosts;
    
    // see https://www.postgresql.org/docs/current/libpq-connect.html#LIBPQ-PARAMKEYWORDS
    std::map<std::string, std::string> m_params;

};

constexpr basic_dtype lookup_dtype(Oid oid)
{
#define X(name) case name ## OID : return basic_dtype::name;

    switch(oid)
    {
        BASIC_DTYPES
        default: throw std::runtime_error(std::string("illegal oid " + std::to_string(oid)));
    }

#undef X
}

constexpr Oid lookup_oid(basic_dtype dtype)
{
#define X(name) case name : return name ## OID;

    switch(dtype)
    {
        BASIC_DTYPES
        default: throw std::runtime_error(std::string("illegal dtype " + std::to_string(static_cast<int>(dtype))));
    }

#undef X
}

constexpr const char* lookup_oid_name(Oid oid)
{
#define X(name) case name ## OID : return #name;

    switch(oid)
    {
        BASIC_DTYPES
        default: throw std::runtime_error(std::string("illegal oid " + std::to_string(oid)));
    }

#undef X
}

constexpr const char* lookup_conversion_typename(basic_dtype dtype)
{
#define X(_name) case _name : return typeid(conversion<_name>::ctype).name();

    switch(dtype)
    {
        BASIC_DTYPES
        default: throw std::runtime_error(std::string("illegal oid " + std::to_string(static_cast<int>(dtype))));
    }

#undef X
}

template<typename T>
constexpr bool is_type(basic_dtype d)
{
#define X(name) case name : return conversion<name>::is<T>();

    switch(d)
    {
        BASIC_DTYPES
        default: throw std::runtime_error(std::string("illegal dtype " + std::to_string(static_cast<int>(d))));
    }

#undef X
}

template<typename T>
constexpr T as_type(basic_dtype d, const std::string& data)
{
#define X(name) case name : return conditional_cast<decltype(conversion<name>::from_pg(data)), T>(conversion<name>::from_pg(data));

    switch(d)
    {
        BASIC_DTYPES
        default: throw std::runtime_error(std::string("illegal dtype " + std::to_string(static_cast<int>(d))));
    }

#undef X
}

struct result_cell
{
    result_cell(std::shared_ptr<PGresult> res, int row, int field)
    {
        m_oid = PQftype(res.get(), field);
        m_null = PQgetisnull(res.get(), row, field);

        int l = PQgetlength(res.get(), row, field);
        char *ptr = PQgetvalue(res.get(), row, field);

        m_data = std::string(ptr, static_cast<std::string::size_type>(l));
    }

    template<typename T>
    bool is()
    {
        return is_type<T>(lookup_dtype(m_oid));
    }

    template<typename T>
    std::optional<T> as()
    {
        return as_type<std::optional<T>>(lookup_dtype(m_oid), m_data);
    }

    template<typename T>
    bool is_array_of()
    {
        return is_type<std::optional<std::vector<std::optional<T>>>>(lookup_dtype(m_oid));
    }

    template<typename T>
    std::optional<std::vector<std::optional<T>>> as_array_of()
    {
        return as_type<std::optional<std::vector<std::optional<T>>>>(lookup_dtype(m_oid), m_data);
    }

    bool is_int()
    {
        return is_type<int>(lookup_dtype(m_oid));
    }

    template<typename int_t = int32_t>
    std::optional<int_t> as_int()
    {
        return as_type<std::optional<int_t>>(lookup_dtype(m_oid), m_data);
    }

    bool is_string()
    {
        return is_type<std::string>(lookup_dtype(m_oid));
    }

    template<typename str_t = std::string>
    std::optional<str_t> as_string()
    {
        return as_type<std::optional<str_t>>(lookup_dtype(m_oid), m_data);
    }

    Oid m_oid;
    bool m_null;
    std::string m_data;
};

struct result
{
    result(): m_res(nullptr) { }

    result(PGresult *res): m_res(res, ::PQclear) { }

    ExecStatusType status() const
    {
        return PQresultStatus(m_res.get());
    }

    std::string error_message() const
    {
        return std::string(PQresultErrorMessage(m_res.get()));
    }

    int ntuples() const
    {
        return PQntuples(m_res.get());
    }

    int nfields() const
    {
        return PQnfields(m_res.get());
    }

    std::string column_name(int idx) const
    {
        char *c = PQfname(m_res.get(), idx);
        return c ? std::string(c) : "";
    }

    bool is_null(int row, int field) const
    {
        return PQgetisnull(m_res.get(), row, field) ? true : false;
    }

    std::optional<std::string> get_value(int row, int field) const
    {
        if(PQgetisnull(m_res.get(), row, field))
        {
            return std::optional<std::string>();
        }

        int l = PQgetlength(m_res.get(), row, field);

        return std::string(PQgetvalue(m_res.get(), row, field), static_cast<std::string::size_type>(l));
    }

    result_cell cell(int row, int field) const
    {
        return result_cell(m_res, row, field);
    }

    std::shared_ptr<PGresult> m_res;
};

static result execute(std::function<PGresult*()> const& fn, std::string const& prefix, std::string const& sql)
{
    auto start = get_time_ns();

    auto pres = fn();

    auto end = get_time_ns();

    spdlog::info("[{0}ms] {1}",
        TO_MS_F(FROM_NS((end - start))),
        prefix + " " + (sql.find('\n') != std::string::npos ? sql : str_trim(sql)));

    auto res = result(pres);

    if(res.status() == PGRES_FATAL_ERROR || res.status() == PGRES_NONFATAL_ERROR)
    {
        spdlog::error("error: {0}", res.error_message());
    }
    
    return res;
}

template <class, class>
struct tuple_concat { };

template <class... First, class... Second>
struct tuple_concat<std::tuple<First...>, std::tuple<Second...>> {
    using type = std::tuple<First..., Second...>;
};

template<typename... Rest>
struct to_optional
{
    typedef std::tuple<> type;
};

template<typename T, typename... Rest>
struct to_optional<T, Rest...>
{
    typedef tuple_concat<std::tuple<std::optional<T>>, typename to_optional<Rest...>::type> type;
};

template<typename... Rest>
struct tuple_extractor
{
    static std::tuple<> extract(const result&, int, int=0)
    {
        return std::tuple<>();
    }
};

template<typename T, typename... Rest>
struct tuple_extractor<T, Rest...>
{
    static auto extract(const result& res, int row, int field=0)
    {
        auto c = res.cell(row, field);
        auto v = c.is<T>() ? c.as<T>() : std::optional<T>();

        return std::tuple_cat(std::tuple<std::optional<T>>(v), tuple_extractor<Rest...>::extract(res, row, field+1));
    }
};

template<typename... dtypes>
auto to_tuples(const result& res)
{
    int n = res.ntuples();

    std::vector<decltype(tuple_extractor<dtypes...>::extract(res, 0))> v;

    for(int i = 0; i < n; ++i)
    {
        v.emplace(std::move(tuple_extractor<dtypes...>::extract(res, i)));
    }

    return v;
}

template<typename... dtypes>
decltype(tuple_extractor<dtypes...>::extract(std::declval<const result&>(), 0)) to_tuple(const result& res, int row)
{
    return tuple_extractor<dtypes...>::extract(res, row);
}

enum value_format : int
{
    PLAIN_TEXT = 0,
    BINARY = 1,
};

struct query_param
{
    template<typename iter>
    query_param(Oid type, value_format format, iter begin, iter end): m_type(type), m_format(format), m_value(begin, end) { }

    query_param(Oid type, value_format format, const std::vector<char>& value): m_type(type), m_format(format), m_value(value) { }

    query_param(Oid type, value_format format, const std::string& value):
        m_type(type), m_format(format), m_value(value.begin(), value.end()) { }

    query_param(basic_dtype type, const std::string& value):
        m_type(lookup_oid(type)), m_format(PLAIN_TEXT), m_value(value.begin(), value.end()) { }

    Oid m_type;
    value_format m_format;
    std::vector<char> m_value;
};

struct prepared_query
{
    prepared_query(std::shared_ptr<PGconn> conn, const std::string &stmtname, bool is_owner, const result& res):
        m_pconn(conn), m_stmtname(stmtname), m_owner(is_owner), m_res(res)
    { }

    ~prepared_query()
    {
        if(m_owner && !m_stmtname.empty())
        {
            PQexec(m_pconn.get(), (std::string("DEALLOCATE ") + m_stmtname).c_str());
        }
    }

    result exec(const std::vector<struct query_param>& param_values = {}, value_format format = value_format::PLAIN_TEXT)
    {
        std::vector<const char*> values;
        std::vector<int> lengths, formats;

        values.reserve(param_values.size());
        lengths.reserve(param_values.size());
        formats.reserve(param_values.size());

        for(const auto& param : param_values)
        {
            values.push_back(param.m_value.data());
            lengths.push_back(param.m_value.size());
            formats.push_back(param.m_format);
        }

        auto start = get_time_ns();

        auto pres = PQexecPrepared(
            m_pconn.get(), m_stmtname.c_str(),
            param_values.size(),
            values.data(), lengths.data(), formats.data(),
            format);

        auto end = get_time_ns();

        spdlog::info("finished executing statement {0} in {1}ms", m_stmtname, TO_MS_F(FROM_NS((end - start))));

        auto res = result(pres);

        if(res.status() == PGRES_FATAL_ERROR || res.status() == PGRES_NONFATAL_ERROR)
        {
            spdlog::error("statement {0} returned error: {1}", m_stmtname, res.error_message());
        }
        else
        {
            spdlog::info("statement {0} status: {1}", m_stmtname, res.error_message());
        }

        return res;
    }

    std::shared_ptr<PGconn> m_pconn;
    std::string m_stmtname;
    bool m_owner;
    result m_res;
};

template<basic_dtype... types>
struct prepared_query2
{
    prepared_query2(std::shared_ptr<PGconn> conn, const std::string &stmtname, bool is_owner, const result& res):
        m_pconn(conn), m_stmtname(stmtname), m_owner(is_owner), m_res(res)
    { }

    prepared_query2(prepared_query2<types...> const& other) = delete;
    prepared_query2(prepared_query2<types...> && other) = default;

    ~prepared_query2()
    {
        if(m_owner && !m_stmtname.empty())
        {
            PQexec(m_pconn.get(), (std::string("DEALLOCATE ") + m_stmtname).c_str());
        }
    }

    result exec(typename conversion<types>::ctype... ctypes, value_format format = value_format::PLAIN_TEXT)
    {
        return exec_tpl(std::make_tuple(ctypes...), format);
    }

    result exec_tpl(const std::tuple<typename conversion<types>::ctype...>& tpl, value_format format = value_format::PLAIN_TEXT)
    {
        std::vector<std::string> strs;
        std::vector<const char*> values;
        std::vector<int> lengths, formats;

        values.reserve(sizeof...(types));
        lengths.reserve(sizeof...(types));
        formats.assign(sizeof...(types), value_format::PLAIN_TEXT);

        convert(strs, tpl);

        for(const auto& s : strs)
        {
            values.push_back(s.data());
            lengths.push_back(s.size());
        }

        return execute([&]()
        {
            return PQexecPrepared(
                m_pconn.get(), m_stmtname.c_str(),
                values.size(),
                values.data(), lengths.data(), formats.data(),
                format);
        }, "[prepared-stmt]", m_stmtname);
    }

    std::shared_ptr<PGconn> m_pconn;
    std::string m_stmtname;
    bool m_owner;
    result m_res;

private:
    template<int N, basic_dtype type>
    static void convert(std::vector<std::string>& v, const std::tuple<typename conversion<types>::ctype...>& v_tpl)
    {
        v[N] = conversion<type>::to_pg(std::get<N>(v_tpl));
    }

    template<std::size_t... Is>
    static void convert(std::vector<std::string>& v, const std::tuple<typename conversion<types>::ctype...>& tpl, std::index_sequence<Is...>)
    {
        v.clear();
        v.resize(sizeof...(types));
        (convert<Is, types>(v, tpl), ...);
    }

    static void convert(std::vector<std::string>& v, const std::tuple<typename conversion<types>::ctype...>& tpl)
    {
        convert(v, tpl, std::index_sequence_for<conversion<types>...>());
    }
};

struct database
{
    database(): m_pconn(nullptr) { }

    database(const connection_info& cinfo)
    {
        open(cinfo);
    }

    database(const std::string& connection_string)
    {
        open(connection_string);
    }

    ~database()
    {
        close();
    }

    void open(const connection_info& cinfo)
    {
        open(cinfo.to_string());
    }

    void open(const std::string& connection_string)
    {
        close();
        m_pconn = std::shared_ptr<PGconn>(PQconnectdb(connection_string.c_str()), ::PQfinish);

        if(PQstatus(m_pconn.get()) != CONNECTION_OK)
        {
            spdlog::error("connection to database failed: {0}", PQerrorMessage(m_pconn.get()));
            throw std::runtime_error(std::string("Connection to database failed:") + PQerrorMessage(m_pconn.get()));
        }
    }

    void close()
    {
        m_pconn = nullptr;
    }

    bool is_open()
    {
        return m_pconn != nullptr;
    }

    std::shared_ptr<prepared_query> prepare(const std::string& cmd, const std::string& name, const std::vector<struct query_param>& param_types = {})
    {
        std::vector<Oid> oids;
        oids.reserve(param_types.size());

        for(const auto& param : param_types)
        {
            oids.push_back(static_cast<Oid>(param.m_format));
        }

        return std::make_shared<prepared_query>(m_pconn, name, true,
            execute([&](){
                return PQprepare(m_pconn.get(), name.c_str(), cmd.c_str(), oids.size(), oids.data());
            }, "[prepare]", cmd));
    }

    std::shared_ptr<prepared_query> prepare(const query& cmd, const std::string& name, const std::vector<struct query_param>& param_types = {})
    {
        return prepare(to_sql(cmd), name, param_types);
    }

    std::shared_ptr<prepared_query> prepare(const query& cmd, const std::vector<struct query_param>& param_types = {})
    {
        return prepare(to_sql(cmd), cmd.m_alias ? cmd.m_alias.value() : get_prepared_query_name(), param_types);
    }

    template<basic_dtype... types>
    std::shared_ptr<prepared_query2<types...>> prepare2(const std::string& cmd, const std::string& name)
    {
        std::vector<Oid> oids = {
            lookup_oid(types)...
        };

        return std::make_shared<prepared_query2<types...>>(m_pconn, name, true,
            execute([&](){
                return PQprepare(m_pconn.get(), name.c_str(), cmd.c_str(), oids.size(), oids.data());
            }, "[prepare]", cmd));
    }

    template<basic_dtype... types>
    std::shared_ptr<prepared_query2<types...>> prepare2(const query& cmd, const std::string& name)
    {
        return prepare2<types...>(to_sql(cmd), name);
    }

    template<basic_dtype... types>
    std::shared_ptr<prepared_query2<types...>> prepare2(const query& cmd)
    {
        return prepare2<types...>(to_sql(cmd), cmd.m_alias ? cmd.m_alias.value() : get_prepared_query_name());
    }

    result exec(const std::string& cmd)
    {
        return execute([&](){return PQexec(m_pconn.get(), cmd.c_str());}, "[exec]", cmd);
    }

    result exec(const query& cmd)
    {
        return exec(to_sql(cmd));
    }

    result execParams(const std::string& cmd, const std::vector<struct query_param>& params = {},
            value_format format = value_format::PLAIN_TEXT)
    {
        std::vector<Oid> oids;
        std::vector<const char*> values;
        std::vector<int> lengths, formats;

        oids.reserve(params.size());
        values.reserve(params.size());
        lengths.reserve(params.size());
        formats.reserve(params.size());

        for(const auto& param : params)
        {
            oids.push_back(param.m_type);
            values.push_back(param.m_value.data());
            lengths.push_back(param.m_value.size());
            formats.push_back(param.m_format);
        }

        return execute([&]()
        {
            return PQexecParams(
                m_pconn.get(),
                cmd.c_str(),
                params.size(), oids.data(), values.data(), lengths.data(), formats.data(),
                format);
        }, "[exec-params]", cmd);
    }

    result execParams(const query& cmd, const std::vector<struct query_param>& params = {},
            value_format format = value_format::PLAIN_TEXT)
    {
        return execParams(to_sql(cmd), params, format);
    }

    template<basic_dtype... types>
    result execParams(const std::string& cmd, typename conversion<types>::ctype... params)
    {
        return execParams(cmd, {
            query_param(lookup_oid(types), PLAIN_TEXT, conversion<types>::to_pg(params))...
        });
    }

    template<basic_dtype... types>
    result execParams(const query& cmd, typename conversion<types>::ctype... params)
    {
        return execParams(to_sql(cmd), {
            query_param(lookup_oid(types), PLAIN_TEXT, conversion<types>::to_pg(params))...
        });
    }

    std::string get_prepared_query_name()
    {
        return "query" + std::to_string(next_pq_id++);
    }

    std::vector<entity_name> get_tables();
    std::vector<entity_name> get_dtypes();
    std::vector<entity_name> get_functions();
    std::vector<entity_name> get_stored_procedures();
    std::vector<entity_name> get_prepared_statements();

    table tbl(const entity_name& name)
    {
        return table(this, name);
    }

    table tbl(const std::string& name)
    {
        return table(this, name);
    }

    dtype type(const entity_name& name)
    {
        return dtype(name);
    }

    dtype type(const std::string& name)
    {
        return dtype(name);
    }

    // class function function(const entity_name& name);

    // class stored_procedure stored_procedure(const entity_name& name);

    // class transaction begin_transaction();

    std::shared_ptr<PGconn> m_pconn;

    int next_pq_id = 0;
};

}
}
