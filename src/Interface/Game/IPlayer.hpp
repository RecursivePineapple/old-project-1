
#pragma once

#include "Common/Types.hpp"

#include "Interface/Server/IConnection.hpp"

#include "Interface/Game/INamedEntity.hpp"

namespace gamestate
{
    struct PlayerAuthInfo;

    struct IPlayer
    {
        virtual ~IPlayer() { }

        PROPERTY(IWorld*, World)
        
        PROPERTY(SP<server::IConnection>, Connection)

        PROPERTY(SP<INamedEntity>, Entity)
        
        PROPERTY(PlayerAuthInfo, AuthInfo)

        virtual bool IsAuthenticated() = 0;
    };
}
