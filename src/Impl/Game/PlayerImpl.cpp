
#pragma target server

#include "Common/Types.hpp"
#include "Common/PlayerAuthInfo.hpp"

#include "Interface/Game/IPlayer.hpp"

namespace gamestate
{
    struct Player : public IPlayer
    {
        virtual SP<server::IConnection> GetConnection() override
        {
            return m_connection;
        }

        virtual void SetConnection(SPCR<server::IConnection> conn) override
        {
            m_connection = conn;
        }

        virtual bool IsAuthenticated() override
        {
            return m_auth_info.is_authenticated;
        }

        virtual CR<PlayerAuthInfo> GetAuthInfo() override
        {
            return m_auth_info;
        }

        virtual void SetAuthInfo(CR<PlayerAuthInfo> info) override
        {
            m_auth_info = info;
        }
    
        virtual void SetEntity(SPCR<INamedEntity> entity) override { m_entity = entity; }

        virtual SP<INamedEntity> GetEntity() override { return m_entity; }

        virtual ISession* GetSession() override
        {
            return m_session;
        }

        virtual void SetSession(ISession* session) override
        {
            m_session = session;

            if(m_current_world != session->GetWorld())
            {
                
            }
        }

    private:
        IWorld *m_current_world;
        ISession* m_session;
        SP<server::IConnection> m_connection;
        PlayerAuthInfo m_auth_info;
        SP<INamedEntity> m_entity;
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
