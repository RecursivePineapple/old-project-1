
#pragma once

#include <string>
#include <cstdlib>

namespace utils
{
    static std::string getenv(const std::string& env_var, const std::string& def)
    {
        const char* val = std::getenv(env_var.data());

        if(val != nullptr)
        {
            return std::string(val);
        }
        else
        {
            return def;
        }
    }
}
