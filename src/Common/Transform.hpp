
#pragma once

#include "Common/Types.hpp"
#include "Common/linalg.hpp"

namespace gamestate
{
    using Location = linalg::aliases::int3;
    using Rotation = linalg::aliases::float3;
    using Scale = linalg::aliases::float3;

    struct Transform
    {
        Location m_loc;
        Rotation m_rot;
        Scale m_scale;

        Transform() { }

        Transform(CR<Location> loc): m_loc(loc) { }

        Transform(CR<Location> loc, CR<Rotation> rot, CR<Scale> scale): m_loc(loc), m_rot(rot), m_scale(scale) { }
    };
}
