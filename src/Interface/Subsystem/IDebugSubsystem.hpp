
#pragma once

#include "Common/Types.hpp"
#include "Interface/Server/ISubsystem.hpp"
#include "Interface/Game/IWorld.hpp"
#include "Interface/Game/ISession.hpp"

namespace gamestate
{
    struct IDebugSubsystem : public server::ISubsystem
    {
        virtual ~IDebugSubsystem() { }
    };
}
