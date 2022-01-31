
#pragma target server

#include <spdlog/spdlog.h>

#include "Common/Types.hpp"

#include "Interface/Subsystem/IDebugSubsystem.hpp"

#include "Interface/Component/IWorldTracker.hpp"

#include "Interface/Game/IPlayer.hpp"

#include "Common/EntityMessage.hpp"

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

        virtual void OnMessage(CR<server::Message> msg) override
        {
            if(msg.Action() == "create-hub")
            {
                IWorld *world = m_world_tracker->CreateHubWorld();

                msg.Sender()->m_player->GetConnection()->SendObject({
                    {"status", "success"},
                    {"world-id", world->GetId().to_string()},
                });
            }
            else if(msg.Action() == "create-entity")
            {
                messages::CreateEntity create;

                if(!msg.ParseInto(create))
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "error"},
                        {"message", "invalid message format"}
                    });
                    return;
                }

                IWorld *world = m_world_tracker->FindWorld(create.world_id);
                spdlog::info("world id: {0}", create.world_id.to_string());

                if(world == nullptr)
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "error"},
                        {"message", "could not find world"}
                    });
                    return;
                }

                auto e = world->Spawn(create.type, INamedEntity::TransformType(world, Location(create.x, create.y, create.z)));

                if(e != nullptr)
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "success"},
                        {"eid", e->GetId().to_string()}
                    });
                }
                else
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "error"}
                    });
                }
            }
        }
    
        SP<IWorldTracker> m_world_tracker;
    };
}

#include <Hypodermic/ContainerBuilder.h>

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
