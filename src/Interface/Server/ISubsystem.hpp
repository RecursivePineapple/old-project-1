
#pragma once

#include "Common/Message.hpp"

namespace server
{
    struct ISubsystem
    {
        virtual ~ISubsystem() { }

        virtual void OnMessage(ConnectionContext *sender, CR<server::Message> msg) = 0;
    
        bool bWantsEntityMessages = false;
        bool bWantsSubsystemMessages = true;
    };
}
