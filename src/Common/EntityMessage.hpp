
#pragma once

#include "Utils/jsonstruct.hpp"
#include "Utils/jsonuuid.hpp"

#include "Common/Message.hpp"
#include "Common/TransformParser.hpp"

namespace gamestate
{
    using StateType = jsontypes::variant<jsontypes::span, server::StringMessage>;

    #define ENTITY_MESSAGE_FIELDS(X) \
        X(std::optional<std::string>, jsontypes::string, action) \
        X(uuid, jsontypes::uuid, dst_id) \
        X(std::optional<Transform>, jsontypes::transform, transform) \
        X(std::optional<StateType::ctype>, StateType, state) \
        X(std::optional<jsontypes::span_t>, jsontypes::span, data)

    struct EntityMessage {
        DECLARE_JSON_STRUCT_BODY(EntityMessage, ENTITY_MESSAGE_FIELDS)

        EntityMessage() { }

        EntityMessage(CR<Transform> t): transform(t) { }

        EntityMessage(CR<std::string> st): state(server::StringMessage(st)) { }

        template<typename TState>
        EntityMessage(CR<TState> value): state(server::StringMessage::FromValue(value)) { }

        EntityMessage(CR<Transform> t, CR<std::string> st): transform(t), state(server::StringMessage(st)) { }
        template<typename TState>
        EntityMessage(CR<Transform> t, CR<TState> value): transform(t), state(server::StringMessage::FromValue(value)) { }
    };

}
