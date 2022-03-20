
#pragma once

#include <string>
#include <vector>

#include "Utils/jsonstruct.hpp"

namespace server
{
    template<typename TParser, typename T>
    static bool ParseValue(const std::vector<jsmntok_t> &tok_buffer, int tok, const std::string &text, const std::string &key, T &value)
    {
        if(tok_buffer[static_cast<size_t>(tok)].type != JSMN_OBJECT) return false;

        ++tok;

        std::string s;
        for(int i = 0; i < tok_buffer[static_cast<size_t>(tok)].size; ++i)
        {
            int key_idx = tok;
            tok = jsontypes::next_token(tok_buffer.data(), tok);
            int value_idx = tok;
            tok = jsontypes::next_token(tok_buffer.data(), tok);

            if(!jsontypes::string::parse(tok_buffer.data(), key_idx, text.c_str(), s)) { continue; }

            if(s != key) { continue; }

            if(!TParser::parse(tok_buffer.data(), value_idx, text.c_str(), value)) { continue; }

            return true;
        }

        return false;
    }

    template<typename T>
    static bool ParseValue(const std::vector<jsmntok_t> &tok_buffer, int tok, const std::string &text, const std::string &key, T &value)
    {
        return ParseValue<T, T>(tok_buffer, tok, text, key, value);
    }
}
