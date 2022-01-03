
#pragma once

#include "Common/Types.hpp"
#include "Interface/Server/ISubsystem.hpp"
#include "Interface/Game/IPlayer.hpp"

namespace server
{
    struct IPlayerSubsystem : ISubsystem
    {
        virtual ~IPlayerSubsystem() { }

        virtual SP<gamestate::IPlayer> CreateUnathenticated() = 0;
        virtual void Disconnect(SPCR<gamestate::IPlayer> player) = 0;
    };
}
