
#pragma once

#include <string>

#include "Utils/uuid.hpp"

namespace gamestate
{
    struct PlayerAuthInfo
    {
        bool m_is_authenticated;
        std::string m_username;
        uuid m_world_id;
        uuid m_entity_id;

        PlayerAuthInfo(
        ) : m_is_authenticated(false) { }

        PlayerAuthInfo(
            std::string username,
            uuid world_id,
            uuid entity_id
        ) : m_is_authenticated(true),
            m_username(username),
            m_world_id(world_id),
            m_entity_id(entity_id) { }
    };
}
