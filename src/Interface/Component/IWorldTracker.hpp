
#pragma once

#include "Utils/uuid.hpp"

#include "Common/Types.hpp"

#include "Interface/Game/IWorld.hpp"

namespace gamestate
{
    struct IWorldTracker
    {
        virtual ~IWorldTracker() { }

        virtual IWorld* FindLoadedWorld(CR<uuid> id) = 0;

        virtual IWorld* FindWorld(CR<uuid> id) = 0;

        virtual IWorld* CreateHubWorld() = 0;
    };
}
