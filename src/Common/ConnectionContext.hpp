#pragma once

#include "Common/Types.hpp"

#include <map>
#include <string>

namespace gamestate
{
    struct IPlayer;
}

namespace server
{
    struct ConnectionContext
    {
        SP<gamestate::IPlayer> m_player;
    };
}
