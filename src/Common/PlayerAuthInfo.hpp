
#pragma once

#include <string>

#include "Utils/uuid.hpp"

namespace gamestate
{
    struct PlayerAuthInfo
    {
        bool is_authenticated;
        std::string username;
        uuid world_id;
        uuid entity_id;
    };
}
