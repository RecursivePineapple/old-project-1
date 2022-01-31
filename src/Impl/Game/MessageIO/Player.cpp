
#pragma target server

#include "Interface/Game/MessageIO/Player.hpp"

namespace gamestate
{
namespace net
{
    void SendLoadWorld(SPCR<server::IConnection> conn, IWorld *world)
    {
        conn->SendObject({
            {"action", "load-world"},
            {"world-type", world->WorldType()},
            {"world-id", world->GetId().to_string()}
        });
    }

    void SendPossess(SPCR<server::IConnection> conn, SPCR<INamedEntity> player_entity)
    {
        conn->SendObject({
            {"action", "load-world"},
            {"world-type", world->WorldType()},
            {"world-id", world->GetId().to_string()}
        });
    }
}
}
