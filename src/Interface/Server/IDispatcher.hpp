
#pragma once

#include <string>
#include <functional>

#include "Common/Types.hpp"
#include "Common/ConnectionContext.hpp"

namespace server
{
    struct IDispatcher
    {
        virtual ~IDispatcher() { }


        virtual bool Dispatch(ConnectionContext *sender, CR<std::string_view> msg_text) = 0;
    };
}
