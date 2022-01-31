
#pragma target server

#include <iomanip>
#include <sstream>

#include "libpq.hpp"
#include "util.hpp"

namespace dbwrapper
{
namespace libpq
{

std::string url_encode(const std::string &value)
{
    std::stringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
    {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << static_cast<int>(static_cast<uint8_t>(c));
        escaped << std::nouppercase;
    }

    return escaped.str();
}

// std::vector<entity_name> table_names()
// {
//     static const char *sql = 
//         "SELECT schemaname, tablename"
//         "FROM pg_catalog.pg_tables"
//         "WHERE schemaname NOT IN ('pg_catalog', 'information_schema')";
    
//     // if(!m_tables)
//     // {

//     // }

//     std::vector<entity_name> x;
//     return x;
// }

std::string connection_info::to_string() const
{
    std::stringstream s;

    s << "postgresql://";

    if(!m_user.empty())
    {
        s << url_encode(m_user);

        if(!m_password.empty())
        {
            s << ":" << url_encode(m_password);
        }

        s << "@";
    }

    stream_join_map(s, ",", m_hosts, [](const host_info &host) {
        return host.to_string();
    });

    s << "/";

    s << url_encode(m_database);

    if(!m_params.empty())
    {
        s << "?";

        stream_join_map(s, "&", m_params, [](const std::pair<std::string, std::string> &pair) {
            return url_encode(pair.first) + "=" + url_encode(pair.second);
        });
    }

    return s.str();
}

std::shared_ptr<prepared_query> prepare(
    const std::shared_ptr<PGconn> &m_pconn,
    const std::string& cmd, const std::vector<struct param_value>& param_types,
    const std::string& name)
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

result exec(
    const std::shared_ptr<PGconn> &m_pconn,
    const std::string& cmd)
{
    return execute([&](){return PQexec(m_pconn.get(), cmd.c_str());}, "[exec]", cmd);
}

result execParams(
    const std::shared_ptr<PGconn> &m_pconn,
    const std::string& cmd, const std::vector<struct param_value>& params,
    value_format format)
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

}
}
