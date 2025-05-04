
#pragma once

#include <future>

#include "Common/Types.hpp"

#include "Interface/Game/IWorld.hpp"
#include "Interface/Game/INamedEntity.hpp"
#include "Interface/Game/IPlayer.hpp"

namespace gamestate
{
    struct IRequestDispatcher
    {
        virtual ~IRequestDispatcher() { }

        virtual std::future<std::optional<std::string>> GetLocalAddress();
    };
}
