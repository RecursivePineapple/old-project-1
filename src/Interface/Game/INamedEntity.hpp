#pragma once

#include "Utils/jsonuuid.hpp"

#include "Common/Types.hpp"
#include "Common/Message.hpp"
#include "Common/WorldLocation.hpp"

#include "Interface/Game/ISession.hpp"

namespace gamestate
{
    struct INamedEntity
    {
        virtual ~INamedEntity() { }

        typedef WorldLocation LocationType;

        virtual std::string EntityType() = 0;

        virtual bool LoadState(CR<std::string> msg) = 0;
        virtual std::string SaveState() = 0;
        virtual std::string SaveViewState() = 0;

        virtual bool IsLoaded() = 0;

        virtual void BeginPlay() { }
        virtual void EndPlay() { }

        virtual ISession* TryLock(ISession *session) = 0;

        virtual bool Unlock() = 0;

        virtual void OnMessage(CR<server::Message> msg) = 0;
    
        virtual CR<uuid> GetId() = 0;
        virtual void SetId(CR<uuid> id) = 0;
        virtual ISession* GetSession() = 0;
        virtual CR<LocationType> GetLocation() = 0;
        virtual void SetLocation(CR<LocationType> loc) = 0;
        virtual IWorld* GetWorld() { return GetLocation().m_world; }
    };
}
