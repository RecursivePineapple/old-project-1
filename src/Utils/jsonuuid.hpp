
#pragma once

#include "Utils/uuid.hpp"
#include "Utils/jsonstruct.hpp"

namespace jsontypes
{
    struct uuid
    {
        typedef ::uuid ctype;
        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            std::string s;
            if(!jsontypes::string::parse(toks, tok, str, s)) { return false; }
            
            return ctype::parse(s, value);
        }
        
        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            return jsontypes::string::emit(s, value.to_string());
        }
    };
}
