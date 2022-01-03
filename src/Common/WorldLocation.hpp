
#pragma once

#include "Utils/vec.hpp"

#include "Common/Types.hpp"
#include "Common/Location.hpp"

namespace gamestate
{
    struct IWorld;

    struct WorldLocation
    {
        IWorld *m_world;

        Location m_loc;

        constexpr vec<3> ToMeters()
        {
            vec<3> v;

            double ratio = 1/1000.0;

            v.values[0] = m_loc.comp.x * ratio;
            v.values[1] = m_loc.comp.y * ratio;
            v.values[2] = m_loc.comp.z * ratio;

            return v;
        }

        WorldLocation() { }

        WorldLocation(IWorld *world, CR<Location> loc) : m_world(world), m_loc(loc) { }

        WorldLocation(IWorld *world, int64_t x, int64_t y, int64_t z) : m_world(world), m_loc(x, y, z) { }

        operator IWorld*()
        {
            return m_world;
        }

        operator CR<Location>()
        {
            return m_loc;
        }
    };
}