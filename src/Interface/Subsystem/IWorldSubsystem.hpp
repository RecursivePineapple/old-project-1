
#pragma once

#include "Common/Types.hpp"
#include "Interface/Server/ISubsystem.hpp"

namespace gamestate
{
    struct IWorldSubsystem : server::ISubsystem
    {
        virtual ~IWorldSubsystem() { }
    };
}
