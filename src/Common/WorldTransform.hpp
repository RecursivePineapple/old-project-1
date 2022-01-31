
#pragma once

#include "Common/Types.hpp"
#include "Common/Transform.hpp"

namespace gamestate
{
    struct IWorld;

    struct WorldTransform
    {
        IWorld *m_world;
        Transform m_transform;

        constexpr linalg::aliases::float3 ToMeters()
        {
            return m_transform.m_loc / 1000.0f;
        }

        WorldTransform() { }

        WorldTransform(IWorld *world, CR<Location> loc) : m_world(world), m_transform(loc) { }

        WorldTransform(IWorld *world, int32_t x, int32_t y, int32_t z) : m_world(world), m_transform(Location{x, y, z}) { }

        operator IWorld*()
        {
            return m_world;
        }

        operator CR<Transform>()
        {
            return m_transform;
        }

        operator CR<Location>()
        {
            return m_transform.m_loc;
        }
    };
}
