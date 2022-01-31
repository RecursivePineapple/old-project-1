
#pragma target server

#include <spdlog/spdlog.h>

#include "Interface/Subsystem/IWorldSubsystem.hpp"
#include "Interface/Game/IWorld.hpp"
#include "Interface/Component/IWorldTracker.hpp"

namespace gamestate
{
    #define WORLD_MESSAGE_FIELDS(X) \
        X(uuid, jsontypes::uuid, dst_id) \
        X(gamestate::EntityMessage, gamestate::EntityMessage, entity)

    DECLARE_JSON_STRUCT(WorldMessage, WORLD_MESSAGE_FIELDS)

    struct WorldSubsystem : public IWorldSubsystem
    {
        WorldSubsystem(
            SPCR<IWorldTracker> tracker
        ) : m_tracker(tracker)
        {
            bWantsSubsystemMessages = false;
            bWantsEntityMessages = true;
        }

        virtual void OnMessage(CR<server::Message> msg) override
        {
            WorldMessage wmsg;

            if(!msg.ParseInto(wmsg))
            {
                spdlog::warn("received invalid world message: {0}", msg.Text());
                return;
            }

            IWorld *world = m_tracker->FindLoadedWorld(wmsg.dst_id);

            if(world == nullptr)
            {
                spdlog::warn("received world message for an unloaded or invalid world: {0}", msg.Text());
                return;
            }

            world->Dispatch(msg.Sender(), wmsg.entity);
        }
    
        SP<IWorldTracker> m_tracker;
    };
}
