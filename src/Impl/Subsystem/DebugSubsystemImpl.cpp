
#pragma target server

#include <spdlog/spdlog.h>

#include "Common/Types.hpp"

#include "Interface/Subsystem/IDebugSubsystem.hpp"

#include "Interface/Component/IWorldTracker.hpp"

#include "Interface/Game/IPlayer.hpp"

#include "Common/Message.hpp"

namespace gamestate
{
    namespace messages
    {
        #define CREATE_ENTITY_FIELDS(X) \
            X(std::string, jsontypes::string, type) \
            X(uuid, jsontypes::uuid, world_id) \
            X(int, jsontypes::integer, x) \
            X(int, jsontypes::integer, y) \
            X(int, jsontypes::integer, z)
        
        DECLARE_JSON_STRUCT(CreateEntity, CREATE_ENTITY_FIELDS)
    }

    struct DebugSubsystem : public IDebugSubsystem
    {
        DebugSubsystem(
            SPCR<IWorldTracker> world_tracker
        ) : m_world_tracker(world_tracker)
            { }

        virtual void OnMessage(ConnectionContext *sender, CR<server::Message> msg) override
        {
            if(msg.action == "create-hub")
            {
                IWorld *world = m_world_tracker->CreateHubWorld();

                sender->m_player->GetConnection()->Send(
                    server::MessageBuilder(server::MessageType::MESSAGE_TYPE_RESPONSE)
                    .data_object({
                        {"world-id", world->GetId()}
                    })
                    .build()
                );
            }
            else if(msg.action == "create-entity")
            {
                messages::CreateEntity create;

                if(!msg.data || !msg.data->ParseInto<messages::CreateEntity>(create))
                {
                    sender->m_player->GetConnection()->Send(
                        server::MessageBuilder(server::MessageType::MESSAGE_TYPE_RESPONSE)
                        .data_object({
                            {"status", "error"},
                            {"message", "invalid message format"}
                        })
                        .build()
                    );
                    return;
                }

                IWorld *world = m_world_tracker->FindWorld(create.world_id);
                spdlog::info("world id: {0}", create.world_id.to_string());

                if(world == nullptr)
                {
                    sender->m_player->GetConnection()->Send(
                        server::MessageBuilder(server::MessageType::MESSAGE_TYPE_RESPONSE)
                        .data_object({
                            {"status", "error"},
                        {"message", "could not find world"}
                        })
                        .build()
                    );
                    return;
                }

                auto e = world->Spawn(create.type, INamedEntity::TransformType(world, create.x, create.y, create.z));

                if(e != nullptr)
                {
                    sender->m_player->GetConnection()->Send(
                        server::MessageBuilder(server::MessageType::MESSAGE_TYPE_RESPONSE)
                        .data_object({
                            {"status", "success"},
                            {"eid", e->GetId().to_string()}
                        })
                        .build()
                    );
                }
                else
                {
                    sender->m_player->GetConnection()->Send(
                        server::MessageBuilder(server::MessageType::MESSAGE_TYPE_RESPONSE)
                        .data_object({
                            {"status", "error"}
                        })
                        .build()
                    );
                }
            }
        }
    
        SP<IWorldTracker> m_world_tracker;
    };
}

#include <Hypodermic/ContainerBuilder.h>

#pragma configurable ConfigureIDebugSubsystem

namespace configure
{
    void ConfigureIDebugSubsystem(Hypodermic::ContainerBuilder &container);
    void ConfigureIDebugSubsystem(Hypodermic::ContainerBuilder &container)
    {
        container
            .registerType<gamestate::DebugSubsystem>()
            .as<gamestate::IDebugSubsystem>()
            .as<server::ISubsystem>()
            .singleInstance();
    }
}
