
#pragma once

#include "Interface/Server/IConnection.hpp"

#include "Interface/Game/INamedEntity.hpp"

namespace gamestate
{
namespace net
{
    void SendCreateEntity(server::IConnection *conn, INamedEntity *entity);
    void SendDestroyEntity(server::IConnection *conn, INamedEntity *entity);
    void SendUpdateEntity(server::IConnection *conn, INamedEntity *entity);
    void SendUpdateEntityPhysics(server::IConnection *conn, INamedEntity *entity);
}
}
