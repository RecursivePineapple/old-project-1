
#pragma once

#include <vector>

#include "Common/Types.hpp"
#include "Common/Message.hpp"

namespace server
{
    struct IConnection
    {
        virtual ~IConnection() { }

        virtual void Send(const Message *msg) = 0;
        virtual void SendBulk(const Message *data, size_t n) = 0;

        template<typename MessageType>
        void Send(MessageType &&msg)
        {
            Message const& m = msg;
            return Send(&m);
        }

        template<typename Container>
        void SendBulk(Container &&msgs)
        {
            return SendBulk(static_cast<Message*>(msgs.data()), msgs.size());
        }

        void SendObject(CR<std::map<std::string, std::string>> obj)
        {
            Send(server::StringMessage::Object(obj));
        }
    };
}
