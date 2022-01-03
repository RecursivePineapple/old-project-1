
#pragma once

#include <unordered_set>

#include "Interface/Game/INamedEntity.hpp"

namespace gamestate
{
    struct IHubSession
    {
        virtual void SetLockedEntities(CR<std::unordered_set<SP<INamedEntity>>> entities) = 0;
    };
}
