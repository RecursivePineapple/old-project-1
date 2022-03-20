
#pragma target server

#include <spdlog/spdlog.h>

#include "Interface/Subsystem/IWorldSubsystem.hpp"
#include "Interface/Game/IWorld.hpp"
#include "Interface/Component/IWorldTracker.hpp"
#include "Interface/Game/IPlayer.hpp"

namespace gamestate
{
    struct WorldSubsystem : public IWorldSubsystem
    {
        WorldSubsystem(
            SPCR<IWorldTracker> tracker
        ) : m_tracker(tracker)
        {
            bWantsSubsystemMessages = false;
            bWantsEntityMessages = true;
        }

        virtual void OnMessage(ConnectionContext *sender, CR<server::Message> msg) override
        {
            if(!sender->m_player->IsAuthenticated())
            {
                spdlog::warn("unathenticated player tried to send entity message (ip={0}, msg={1})",
                    sender->m_remote_address,
                    msg.to_string());
                return;
            }

            if(!msg.world_id)
            {
                spdlog::warn("received invalid world message (pid={0}, msg={1})",
                    sender->m_player->GetEntity()->GetId().to_string(),
                    msg.to_string()); // TODO
                return;
            }

            IWorld *world = m_tracker->FindLoadedWorld(msg.world_id.value());

            if(world == nullptr)
            {
                spdlog::warn("received world message for an unloaded or invalid world: {0}", "");
                return;
            }

            world->Dispatch(sender, msg);
        }
    
        SP<IWorldTracker> m_tracker;
    };
}
