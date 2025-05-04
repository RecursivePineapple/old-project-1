
#pragma once

#include "Common/Message.hpp"

#include "Interface/Server/IConnection.hpp"

namespace gamestate
{
namespace net
{
    void SendSubsystemAction(ConnectionContext *sender, CR<std::string> action, int status);
    void SendSubsystemAction(ConnectionContext *sender, CR<std::string> action, int status, CR<server::Message::ObjectType::ctype> data);
    void SendResponseAction(ConnectionContext *sender, CR<std::string> action, int status);
    void SendResponseAction(ConnectionContext *sender, CR<std::string> action, int status, CR<server::Message::ObjectType::ctype> data);
}
}
