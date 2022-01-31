
#pragma once

#include "Common/Types.hpp"

#include <string>
#include <vector>
#include <optional>
#include <sstream>

#include "Utils/jsonstruct.hpp"
#include "Utils/jsonparser.hpp"
#include "Utils/jsonuuid.hpp"

#include "Common/ConnectionContext.hpp"

namespace server
{
    enum MessageType
    {
        MESSAGE_TYPE_UNKNOWN = 0,
        MESSAGE_TYPE_SUBSYSTEM_ACTION,
        MESSAGE_TYPE_ENTITY_ACTION,
        MESSAGE_TYPE_ENTITY_UPDATE
    };

    #define MESSAGE_FIELDS(X) \
        X(int, jsontypes::integer, type) \
        X(std::string, jsontypes::string, action) \
        X(std::optional<std::string>, jsontypes::string, target) \
        X(std::optional<uuid>, jsontypes::uuid, eid) \
        X(std::optional<jsontypes::span_t>, jsontypes::span, transform) \
        X(std::optional<jsontypes::span_t>, jsontypes::span, state) \
        X(std::optional<jsontypes::span_t>, jsontypes::span, data)

    DECLARE_JSON_STRUCT(Message, MESSAGE_FIELDS)

    struct Message
    {
        virtual CR<std::string> Action() const = 0;
        virtual CR<std::string> DestId() const = 0;

        virtual CR<std::vector<jsmntok_t>> Tokens() const = 0;

        virtual int TokenIdx() const = 0;

        virtual CR<std::string> Text() const = 0;

        virtual ConnectionContext* Sender() const = 0;

        template<typename T>
        bool ParseInto(typename T::ctype &value) const
        {
            return T::parse(Tokens(), TokenIdx(), Text().data(), value);
        }

        template<typename T>
        bool ParseInto(T &value) const
        {
            return T::parse(Tokens().data(), TokenIdx(), Text().data(), value);
        }

        template<typename T, typename ctype>
        bool GetValue(CR<std::string> key, ctype &value) const
        {
            return server::ParseValue<T, ctype>(Tokens(), TokenIdx(), Text().data(), key, value);
        }
    };

    struct NetworkMessage : Message
    {
        template<typename t1, typename t2, typename t3>
        NetworkMessage(
            t1 &&action, t2 &&dst_id,
            CR<std::vector<jsmntok_t>> tokens, int token_idx,
            t3 &&text,
            ConnectionContext* sender
        ) : m_action(std::move(action)),
            m_dst_id(std::move(dst_id)),
            m_tokens(tokens),
            m_token_idx(token_idx),
            m_text(std::move(text)),
            m_sender(sender)
            { }

        virtual ~NetworkMessage() { }

        virtual CR<std::string> Action() const override { return m_action; }
        virtual CR<std::string> DestId() const override { return m_dst_id; }

        virtual CR<std::vector<jsmntok_t>> Tokens() const override { return m_tokens; }

        virtual int TokenIdx() const override { return m_token_idx; }

        virtual CR<std::string> Text() const override { return m_text; }

        virtual ConnectionContext* Sender() const override { return m_sender; }

        std::string m_action;
        std::string m_dst_id;
        CR<std::vector<jsmntok_t>> m_tokens;
        int m_token_idx;
        std::string m_text;
        ConnectionContext* m_sender;
    };

    struct StringMessage : Message
    {
        typedef StringMessage ctype;
        StringMessage(CR<std::string> text) : m_text(text) { }
        StringMessage(std::string&& text) : m_text(text) { }

        virtual ~StringMessage() { }

        virtual CR<std::string> Action() const override { LoadTokens(); return m_action; }

        virtual CR<std::string> DestId() const override { LoadTokens(); return m_dst_id; }

        virtual CR<std::vector<jsmntok_t>> Tokens() const override { LoadTokens();  return m_tokens; }

        virtual int TokenIdx() const override { return 0; }

        virtual CR<std::string> Text() const override { return m_text; }

        virtual ConnectionContext* Sender() const override { return nullptr; }

        void LoadTokens() const
        {
            if(!m_tokens_loaded)
            {
                if(!ParseJson(m_tokens, m_text))
                {
                    throw std::runtime_error("Attempted to parse invalid json in StringMessage: '" + m_text + "'");
                }
                
                ParseValue<jsontypes::string>(Tokens(), 0, m_text, "action", m_action);
                ParseValue<jsontypes::string>(Tokens(), 0, m_text, "dst_id", m_dst_id);

                m_tokens_loaded = true;
            }
        }

        std::string m_text;

        mutable bool m_tokens_loaded;
        mutable std::vector<jsmntok_t> m_tokens;
        mutable std::string m_action, m_dst_id;

        template<typename T>
        static StringMessage FromValue(CR<typename T::ctype> value)
        {
            std::stringstream ss;

            T::emit(ss, value);

            return StringMessage(ss.str());
        }

        template<typename T>
        static StringMessage FromValue(CR<T> value)
        {
            std::stringstream ss;

            T::emit(ss, value);

            return StringMessage(ss.str());
        }

        static StringMessage Object(CR<std::map<std::string, std::string>> obj)
        {
            typedef jsontypes::object<jsontypes::string, jsontypes::string> object;
            
            return server::StringMessage::FromValue<object>(obj);
        }

        template<typename stream>
        static stream& emit(stream &s, CR<StringMessage>)
        {
            return s;
        }
    };

}
