
#pragma once

#include <vector>

#include "Common/Types.hpp"
#include "Common/Location.hpp"
#include "Common/Message.hpp"

namespace gamestate
{
    struct IPlayer;
    struct INamedEntity;
    struct IWorld;

    struct ISession
    {
        virtual ~ISession() { }

        virtual void AddPlayer(SPCR<IPlayer> player) = 0;
        virtual void RemovePlayer(SPCR<IPlayer> player) = 0;

        virtual void OnPlayerLocationChanged(SPCR<IPlayer> player) = 0;

        virtual void Broadcast(CR<server::Message> msg) = 0;
        
        virtual IWorld* GetWorld() = 0;
        virtual void SetWorld(IWorld *world) = 0;

    };
}
