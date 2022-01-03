
#pragma target server

#include <spdlog/spdlog.h>

#include "Common/Types.hpp"

#include "Interface/Subsystem/IDebugSubsystem.hpp"

#include "Interface/Component/IWorldTracker.hpp"

#include "Interface/Game/IPlayer.hpp"

#include "Common/EntityMessage.hpp"

namespace gamestate
{
    struct DebugSubsystem : public IDebugSubsystem
    {
        DebugSubsystem(
            SPCR<IWorldTracker> world_tracker
        ) : m_world_tracker(world_tracker)
            { }

        virtual void OnMessage(CR<server::Message> msg) override
        {
            if(msg.Type() == "create-hub")
            {
                IWorld *world = m_world_tracker->CreateHubWorld();

                msg.Sender()->m_player->GetConnection()->SendObject({
                    {"status", "success"},
                    {"world-id", world->GetId().to_string()},
                });
            }
            else if(msg.Type() == "create-entity")
            {
                EntityMessage em;

                msg.ParseInto(em);

                std::string type;
                if(!msg.GetValue<jsontypes::string>("type", type))
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "error"},
                        {"message", "invalid entity type"}
                    });
                    return;
                }

                uuid world_id;
                if(!msg.GetValue<jsontypes::uuid>("world-id", world_id))
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "error"},
                        {"message", "invalid world id"}
                    });
                    return;
                }

                IWorld *world = m_world_tracker->FindWorld(world_id);

                auto e = world->Spawn(type, INamedEntity::LocationType(world, Location(em.x, em.y, em.z)));

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
