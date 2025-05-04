
#pragma target server

#include "Utils/http.hpp"

namespace utils
{
    std::optional<jsontypes::span_t> DoGet(CR<std::string> url)
    {
        cURLpp::Easy req;
        std::stringstream ss;
        req.setOpt(cURLpp::Options::Url(url));
        req.setOpt(cURLpp::Options::ReadStream(&ss));

        try
        {
            spdlog::info("GET {0}", url);
            req.perform();
        }
        catch(CR<cURLpp::RuntimeError> e)
        {
            spdlog::error("cURLpp::RuntimeError What='{0}'", e.std::exception::what());
            return std::optional<jsontypes::span_t>();
        }
        catch(CR<cURLpp::LogicError> e)
        {
            spdlog::error("cURLpp::LogicError What='{0}'", e.std::exception::what());
            return std::optional<jsontypes::span_t>();
        }

        std::vector<jsmntok_t> toks;
        if(!jsontypes::ParseJson(toks, ss.str()))
        {
            spdlog::info("GET {0} sent invalid json", url);
            return std::optional<jsontypes::span_t>();
        }

        return jsontypes::span_t(toks, ss.str());
    }

}
