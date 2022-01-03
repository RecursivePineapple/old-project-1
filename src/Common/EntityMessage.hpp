
#pragma once

#include "Utils/jsonstruct.hpp"
#include "Utils/jsonuuid.hpp"

namespace gamestate
{
    using EntityMessageState = jsontypes::object<jsontypes::string, jsontypes::span>;

    enum class EntityMessageActions : int
    {
        Unknown = 0,
        Create = 1,
        Update = 2,
        Remove = 3
    };

    #define ENTITY_MESSAGE_FIELDS(X) \
        X(uuid, jsontypes::uuid, eid) \
        X(int, jsontypes::integer, x) \
        X(int, jsontypes::integer, y) \
        X(int, jsontypes::integer, z) \
        X(EntityMessageState::ctype, EntityMessageState, state) \
        X(EntityMessageState::ctype, EntityMessageState, viewstate)

    DECLARE_JSON_STRUCT(EntityMessage, ENTITY_MESSAGE_FIELDS)

}
