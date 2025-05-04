
#pragma target server

#include <functional>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <spdlog/spdlog.h>

#include "Utils/jsonstruct.hpp"
#include "Utils/jsonparser.hpp"

#include "Common/Types.hpp"
#include "Common/Message.hpp"

#include "Interface/Server/IDispatcher.hpp"
#include "Interface/Server/ISubsystem.hpp"

namespace server
{

    #define ACTION_FIELDS(X) \
        X(std::string, jsontypes::string, action) \
        X(std::string, jsontypes::string, dst_id)

    DECLARE_JSON_STRUCT(action_message, ACTION_FIELDS)

    struct Dispatcher : public IDispatcher
    {
        Dispatcher(std::vector<SP<ISubsystem>> subsystems)
        {
            for(auto const& subsystem : subsystems)
            {
                if(subsystem->bWantsSubsystemMessages)
                {
                    m_sub_subsystems.push_back(subsystem);
                }
                if(subsystem->bWantsEntityMessages)
                {
                    m_entity_subsystems.push_back(subsystem);
                }
            }
        }

        virtual bool Dispatch(ConnectionContext *sender, CR<std::string_view> msg_text) override
        {
            std::vector<jsmntok_t> tok_buffer;

            if(!jsontypes::ParseJson(tok_buffer, msg_text))
            {
                return false;
            }

            if(tok_buffer[0].type == JSMN_ARRAY)
            {
                jsontypes::array<jsontypes::span>::ctype data;
                if(!jsontypes::array<jsontypes::span>::parse(tok_buffer.data(), 0, msg_text.data(), data))
                {
                    spdlog::debug("could not dispatch: bulk message was invalid");
                    return false;
                }

                for(auto const& span : data)
                {
                    if(!dispatch(sender, span.str, span.toks, 0)) { return false; }
                }

                return true;
            }
            else
            {
                return dispatch(sender, msg_text, tok_buffer, 0);
            }
        }

    private:
        bool dispatch(ConnectionContext *sender, CR<std::string_view> msg_text, CR<std::vector<jsmntok_t>> tok_buffer, int tok_idx)
        {
            server::Message msg;
            if(!Message::parse(tok_buffer.data(), tok_idx, msg_text.data(), msg))
            {
                return false;
            }

            switch(msg.type)
            {
                case server::MESSAGE_TYPE_SUBSYSTEM_ACTION: {
                    spdlog::debug("dispatching subsystem message");
                    for(auto const& subsystem : m_sub_subsystems)
                    {
                        subsystem->OnMessage(sender, msg);
                    }

                    break;
                }

                case server::MESSAGE_TYPE_ENTITY_ACTION: {
                    spdlog::debug("dispatching entity message");
                    for(auto const& subsystem : m_entity_subsystems)
                    {
                        subsystem->OnMessage(sender, msg);
                    }

                    break;
                }

                default: {
                    spdlog::debug("received invalid message type: {0}", msg.type);
                    break;
                }
            }

            return true;
        }

        std::vector<SP<ISubsystem>> m_sub_subsystems, m_entity_subsystems;
    };

}

#include <Hypodermic/ContainerBuilder.h>

#pragma configurable ConfigureIDispatcher

namespace configure
{
    void ConfigureIDispatcher(Hypodermic::ContainerBuilder &container);
    void ConfigureIDispatcher(Hypodermic::ContainerBuilder &container)
    {
        container.registerType<server::Dispatcher>().as<server::IDispatcher>().singleInstance();
    }
}
