
#pragma once

#include "Interface/Server/IConnection.hpp"

#include "Interface/Game/IWorld.hpp"
#include "Interface/Game/INamedEntity.hpp"

namespace gamestate
{
namespace net
{
    void SendLoadWorld(SPCR<server::IConnection> conn, IWorld *world);

    void SendPossess(SPCR<server::IConnection> conn, SPCR<INamedEntity> player_entity);
}
}
