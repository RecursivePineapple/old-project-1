
#pragma target server

#include "Interface/Component/IUpdateTransmitter.hpp"
#include "Interface/Game/MessageIO/Entity.hpp"
#include "Interface/Game/MessageIO/Player.hpp"

namespace gamestate
{
    struct UpdateTransmitter : public IUpdateTransmitter
    {
        virtual void SendUpdate(IWorld *world, PTR<IPlayer> player, PTR<INamedEntity> entity) override
        {
            if(ShouldSend(world, player, entity))
            {
                net::SendUpdateEntity(player->GetConnection().get(), entity);
            }
        }

        virtual void SendUpdatePhysics(IWorld *world, PTR<IPlayer> player, PTR<INamedEntity> entity) override
        {
            if(ShouldSend(world, player, entity))
            {
                net::SendUpdateEntityPhysics(player->GetConnection().get(), entity);
            }
        }

        virtual void SendCreate(IWorld *world, PTR<IPlayer> player, PTR<INamedEntity> entity) override
        {
            if(ShouldSend(world, player, entity))
            {
                net::SendCreateEntity(player->GetConnection().get(), entity);
            }
        }

        virtual void SendDestroy(IWorld *world, PTR<IPlayer> player, PTR<INamedEntity> entity) override
        {
            if(ShouldSend(world, player, entity))
            {
                net::SendDestroyEntity(player->GetConnection().get(), entity);
            }
        }

        bool ShouldSend(IWorld *, IPlayer *player, INamedEntity *entity)
        {
            constexpr float view_dist = 100 * 1000;

            return linalg::length2(
                player->GetEntity()->GetLocation().m_transform.m_loc -
                entity->GetLocation().m_transform.m_loc) <= view_dist * view_dist;
        }
    };
}

#include <Hypodermic/ContainerBuilder.h>

#pragma configurable ConfigureIUpdateTransmitter

namespace configure
{
    void ConfigureIUpdateTransmitter(Hypodermic::ContainerBuilder &container);
    void ConfigureIUpdateTransmitter(Hypodermic::ContainerBuilder &container)
    {
        container.registerType<gamestate::UpdateTransmitter>().as<gamestate::IUpdateTransmitter>();
    }
}
