
#pragma once

#include "Common/Types.hpp"

#include "Interface/Game/IWorld.hpp"
#include "Interface/Game/INamedEntity.hpp"
#include "Interface/Game/IPlayer.hpp"

namespace gamestate
{
    struct IUpdateTransmitter
    {
        virtual ~IUpdateTransmitter() { }

        virtual void SendUpdate(IWorld *world, PTR<IPlayer> player, PTR<INamedEntity> entity) = 0;

        virtual void SendUpdatePhysics(IWorld *world, PTR<IPlayer> player, PTR<INamedEntity> entity) = 0;

        virtual void SendCreate(IWorld *world, PTR<IPlayer> player, PTR<INamedEntity> entity) = 0;

        virtual void SendDestroy(IWorld *world, PTR<IPlayer> player, PTR<INamedEntity> entity) = 0;
    };
}
