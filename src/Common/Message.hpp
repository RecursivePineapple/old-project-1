
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
        MESSAGE_TYPE_SUBSYSTEM_ACTION = 1,
        MESSAGE_TYPE_ENTITY_ACTION = 2,
        MESSAGE_TYPE_ENTITY_UPDATE = 3,
        MESSAGE_TYPE_RESPONSE = 4,
    };

    #define MESSAGE_FIELDS(X) \
        X(int, jsontypes::integer, type) \
        X(std::optional<std::string>, jsontypes::string, action) \
        X(std::optional<std::string>, jsontypes::string, target) \
        X(std::optional<uuid>, jsontypes::uuid, world_id) \
        X(std::optional<uuid>, jsontypes::uuid, id) \
        X(std::optional<jsontypes::span_t>, jsontypes::span, transform) \
        X(std::optional<jsontypes::span_t>, jsontypes::span, state) \
        X(std::optional<jsontypes::span_t>, jsontypes::span, data)

    /**
    type: message type
    action: requested action to execute
    target: destination subsystem
    world_id: the world the entity is in
    id: destination entity
    transform: entity transform (server -> client only)
    state: entity state (server -> client only)
    data: any data relevant to message
    */

    struct Message
    {
        DECLARE_JSON_STRUCT_BODY(Message, MESSAGE_FIELDS)

        using ObjectType = jsontypes::object<
            jsontypes::variant<
                jsontypes::integer,
                jsontypes::string
            >,
            jsontypes::variant<
                jsontypes::integer,
                jsontypes::floating,
                jsontypes::uuid,
                jsontypes::boolean,
                jsontypes::string
            >
        >;
        
    };

    struct MessageBuilder
    {
        Message msg;

        MessageBuilder(MessageType type)
        {
            msg.type = type;
        }

        Message build()
        {
            return msg;
        }

        #define X(dtype, name) \
        MessageBuilder& name(CR<dtype> name) \
        { \
            msg.name = name; \
            return *this; \
        }

        #define X2(dtype, name) \
        MessageBuilder& name ## _object(CR<dtype::ctype> name) \
        { \
            msg.name = jsontypes::span_t::FromValue<dtype>(name); \
            return *this; \
        }

        X(std::string, action)
        X(std::string, target)
        X(uuid, world_id)
        X(uuid, id)

        X(jsontypes::span_t, transform)
        X(jsontypes::span_t, state)
        X(jsontypes::span_t, data)
        
        X2(Message::ObjectType, state)
        X2(Message::ObjectType, data)

        #undef X
        #undef X2

    };
}
