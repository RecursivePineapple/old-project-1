
#pragma target server

#include "Utils/jsonuuid.hpp"

#include "Interface/Game/MessageIO/Player.hpp"

namespace gamestate
{
namespace net
{
    void SendLoadWorld(SPCR<server::IConnection> conn, IWorld *world)
    {
        conn->Send(server::MessageBuilder(server::MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION).action("load-world").data({
            {"world-type", world->WorldType()},
            {"world-id", world->GetId()}
        }).build());
    }

    void SendPossess(SPCR<server::IConnection> conn, SPCR<INamedEntity> player_entity)
    {
        conn->Send(server::MessageBuilder(server::MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION).action("possess").data({
            {"entity-id", player_entity->GetId()},
        }).build());
    }
}
}
