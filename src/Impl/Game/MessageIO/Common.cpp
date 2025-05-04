
#pragma target server

#include <spdlog/spdlog.h>

#include "Interface/Game/MessageIO/Common.hpp"

#include "Interface/Game/IPlayer.hpp"

namespace gamestate
{
namespace net
{
    using namespace server;
    void SendSubsystemAction(ConnectionContext *sender, CR<std::string> action, int status)
    {
        const auto& msg = MessageBuilder(MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION)
            .action(action)
            .data_object({
                {"status", status}
            })
            .build();

        sender->m_player->GetConnection()->Send(msg);
        spdlog::debug("[client: {0}] SendSubsystemAction action: {1} status: {2}",
            sender->m_remote_address,
            action,
            status);
    }

    void SendSubsystemAction(ConnectionContext *sender, CR<std::string> action, int status, CR<Message::ObjectType::ctype> data)
    {
        Message::ObjectType::ctype d = data;
        d["status"] = status;

        const auto& msg = MessageBuilder(MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION)
            .action(action)
            .data_object(d)
            .build();

        sender->m_player->GetConnection()->Send(msg);

        std::stringstream ss;
        Message::ObjectType::emit(ss, d);

        spdlog::debug("[client: {0}] SendSubsystemAction action: {1} status: {2} data: {3}",
            sender->m_remote_address,
            action,
            status,
            ss.str());
    }

    void SendResponseAction(ConnectionContext *sender, CR<std::string> action, int status)
    {
        const auto& msg = MessageBuilder(MessageType::MESSAGE_TYPE_RESPONSE)
            .action(action)
            .data_object({
                {"status", status}
            })
            .build();

        sender->m_player->GetConnection()->Send(msg);
        spdlog::debug("[client: {0}] SendResponseAction action: {1} status: {2}",
            sender->m_remote_address,
            action,
            status);
    }

    void SendResponseAction(ConnectionContext *sender, CR<std::string> action, int status, CR<Message::ObjectType::ctype> data)
    {
        Message::ObjectType::ctype d = data;
        d["status"] = status;

        const auto& msg = MessageBuilder(MessageType::MESSAGE_TYPE_RESPONSE)
            .action(action)
            .data_object(d)
            .build();

        sender->m_player->GetConnection()->Send(msg);

        std::stringstream ss;
        Message::ObjectType::emit(ss, d);

        spdlog::debug("[client: {0}] SendResponseAction action: {1} status: {2} data: {3}",
            sender->m_remote_address,
            action,
            status,
            ss.str());
    }
}
}
