
#pragma target server

#include "Interface/Game/MessageIO/Entity.hpp"

namespace gamestate
{
namespace net
{
    void SendCreateEntity(server::IConnection *conn, INamedEntity *entity)
    {
        (void)conn;
        (void)entity;
    }

    void SendDestroyEntity(server::IConnection *conn, INamedEntity *entity)
    {
        (void)conn;
        (void)entity;
    }

    void SendUpdateEntity(server::IConnection *conn, INamedEntity *entity)
    {
        (void)conn;
        (void)entity;
    }

    void SendUpdateEntityPhysics(server::IConnection *conn, INamedEntity *entity)
    {
        (void)conn;
        (void)entity;
    }
}
}
