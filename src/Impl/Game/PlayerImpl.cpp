
#pragma target server

#include "Common/Types.hpp"
#include "Common/PlayerAuthInfo.hpp"

#include "Interface/Game/IPlayer.hpp"

namespace gamestate
{
    struct Player : public IPlayer
    {
        IMPLEMENT_PROPERTY(IWorld*, World, m_world)
        
        IMPLEMENT_PROPERTY(SP<server::IConnection>, Connection, m_connection)

        IMPLEMENT_PROPERTY(SP<INamedEntity>, Entity, m_entity)
        
        IMPLEMENT_PROPERTY(PlayerAuthInfo, AuthInfo, m_auth_info)

        virtual bool IsAuthenticated() override
        {
            return m_auth_info.m_is_authenticated;
        }

    };
}

#include <Hypodermic/ContainerBuilder.h>

namespace configure
{
    void ConfigureIPlayer(Hypodermic::ContainerBuilder &container);
    void ConfigureIPlayer(Hypodermic::ContainerBuilder &container)
    {
        container.registerType<gamestate::Player>().as<gamestate::IPlayer>();
    }
}
