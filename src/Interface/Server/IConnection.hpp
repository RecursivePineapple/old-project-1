
#pragma once

#include <vector>

#include "Common/Types.hpp"
#include "Common/Message.hpp"

namespace server
{
    struct IConnection
    {
        virtual ~IConnection() { }

        virtual void SendUnsafe(CR<std::string> msg) = 0;

        template<typename jtype>
        void SendUnsafe(CR<typename jtype::ctype> x)
        {
            std::stringstream ss;
            jtype::emit(ss, x);
            SendUnsafe(ss.str());
        }

        void Send(CR<Message> msg)
        {
            std::stringstream ss;
            Message::emit(ss, msg);
            SendUnsafe(ss.str());
        }

        void SendBulk(CR<std::vector<Message>> msgs)
        {
            SendUnsafe<jsontypes::array<Message>>(msgs);
        }

        void SendBulk(CR<std::initializer_list<Message>> msgs)
        {
            SendUnsafe<jsontypes::array<Message>>(std::vector<Message>(msgs.begin(), msgs.end()));
        }
    };
}
