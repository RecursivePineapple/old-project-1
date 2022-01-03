
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

}
}
