
#pragma once

#include "Common/Types.hpp"

#include "Interface/Server/IConnection.hpp"

#include "Interface/Game/ISession.hpp"
#include "Interface/Game/INamedEntity.hpp"

namespace gamestate
{
    struct PlayerAuthInfo;

    struct IPlayer
    {
        virtual ~IPlayer() { }

        virtual ISession* GetSession() = 0;
        virtual void SetSession(ISession *session) = 0;

        virtual SP<server::IConnection> GetConnection() = 0;
        virtual void SetConnection(SPCR<server::IConnection> conn) = 0;

        virtual void SetEntity(SPCR<INamedEntity> entity) = 0;
        virtual SP<INamedEntity> GetEntity() = 0;

        virtual bool IsAuthenticated() = 0;
        virtual CR<PlayerAuthInfo> GetAuthInfo() = 0;
        virtual void SetAuthInfo(CR<PlayerAuthInfo> info) = 0;
    };
}
