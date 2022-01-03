
#pragma once

#include <uWebSockets/App.h>

#include "Common/ConnectionContext.hpp"

namespace server
{
    struct IServer
    {
        using SocketType = uWS::WebSocket<false, true, ConnectionContext>;
        using BehaviorType = uWS::TemplatedApp<false>::WebSocketBehavior<ConnectionContext>;

        virtual ~IServer() { }

        virtual BehaviorType Behavior() = 0;
    };
}
