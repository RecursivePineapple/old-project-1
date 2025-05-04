
#pragma target server

#include <Hypodermic/Container.h>
#include <spdlog/spdlog.h>

#include "Utils/dbwrapper/libpq.hpp"
#include "Utils/queryhelper.hpp"

#include "Common/Types.hpp"
#include "Common/PlayerAuthInfo.hpp"
#include "Common/ErrorCodes.hpp"

#include "Interface/Subsystem/IPlayerSubsystem.hpp"
#include "Interface/Component/IWorldTracker.hpp"
#include "Interface/Component/IRequestDispatcher.hpp"
#include "Interface/Game/MessageIO/Common.hpp"

/* #region Queries */

#define FIND_SESSION_PARAMS(X) \
    X(UUID, session)

#define LOCK_SESSION_PARAMS(X) \
    X(UUID, session) \
    X(VARCHAR, host_addr)

#define UNLOCK_SESSION_PARAMS(X) \
    X(UUID, session) \
    X(VARCHAR, host_addr)

#define FIND_SESSION_QUERY(param) \
    pq::table tbl("session"); \
    std::string sql = tbl.select( \
        tbl.col("player_id"), tbl.col("client_addr"), tbl.col("host_addr") \
    ).where( \
        tbl.col("session_id") == param(session) \
    );

#define LOCK_SESSION_QUERY(param) \
    pq::table tbl("session"); \
    std::string sql = tbl.update().set(tbl.col("host_addr"), param(host_addr)) \
        .where(tbl.col("host_addr") == nullptr && tbl.col("session_id") == param(session));

#define QUERIES_X(X) \
  X(FindSession, FIND_SESSION_PARAMS, FIND_SESSION_QUERY) \
  X(LockSession, LOCK_SESSION_PARAMS, LOCK_SESSION_QUERY)

/* #endregion */

namespace server
{
    #define PLAYER_AUTH_FIELDS(X) \
        X(uuid, jsontypes::uuid, session_id)

    DECLARE_JSON_STRUCT(PlayerAuthMsg, PLAYER_AUTH_FIELDS)

    namespace pq = dbwrapper::libpq;
    QUERY_STRUCT_DECL(PlayerQueries, QUERIES_X)

    struct PlayerSubsystem : public IPlayerSubsystem
    {
        PlayerSubsystem(
            SPCR<Hypodermic::Container> container,
            SPCR<gamestate::IWorldTracker> world_tracker,
            SPCR<pq::database> db,
            SPCR<gamestate::IRequestDispatcher> req
        ) : m_container(container),
            m_world_tracker(world_tracker),
            m_db(db),
            m_req(req)
            { }
        
        void InitDB() { m_queries = PlayerQueries(m_db); }

        virtual SP<gamestate::IPlayer> CreateUnathenticated() override
        {
            return m_container->resolve<gamestate::IPlayer>();
        }

        virtual void Disconnect(SPCR<gamestate::IPlayer> player) override
        {
            
        }

        virtual void OnMessage(ConnectionContext *sender, CR<server::Message> msg) override
        {
            if(msg.action == "set-session")
            {
                PlayerAuthMsg auth;

                if(!msg.data || !msg.data->ParseInto<PlayerAuthMsg>(auth))
                {
                    gamestate::net::SendSubsystemAction(sender, "invalid-session", error::Session::InvalidFormat);
                    return;
                }

                auto res = m_queries.FindSession(auth.session_id);

                if(res.ntuples() != 1)
                {
                    gamestate::net::SendSubsystemAction(sender, "invalid-session", error::Session::MissingSession);
                    return;
                }

                auto tpl = pq::to_nonoptional_tuple<uuid, std::string, std::optional<std::string>>(res, 0);

                if(!tpl)
                {
                    gamestate::net::SendSubsystemAction(sender, "invalid-session", error::Session::InvalidSession);
                    return;
                }

                auto [player_id, client_addr, host_addr] = tpl.value();

                if(client_addr != sender->m_remote_address)
                {
                    gamestate::net::SendSubsystemAction(sender, "invalid-session", error::Session::InvalidSessionClient);
                    return;
                }

                if(host_addr)
                {
                    gamestate::net::SendSubsystemAction(sender, "invalid-session", error::Session::SessionAlreadyRunning);
                    return;
                }

                auto hostname_req = m_req->GetLocalAddress();
                if(hostname_req.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
                {
                    gamestate::net::SendSubsystemAction(sender, "invalid-session", error::Session::AddrFetchTimeout);
                    return;
                }

                auto hostname = hostname_req.get();

                if(!hostname)
                {
                    gamestate::net::SendSubsystemAction(sender, "invalid-session", error::Session::AddrFetchError);
                    return;
                }

                spdlog::debug("player authenticated as {0} with world_id {1} and entity_id {2}",);
                m_queries.LockSession(auth.session_id, );

                sender->m_player->SetAuthInfo(gamestate::PlayerAuthInfo(auth.username, world_id, entity_id));

                gamestate::net::SendSubsystemAction(sender, "valid-session", error::Success);
                spdlog::debug("player authenticated as {0} with world_id {1} and entity_id {2}",
                    auth.session_id,
                    player_id.to_string(),
                    entity_id.to_string());

                OnPlayerLoggedIn(sender->m_player);
            }
        }

        void OnPlayerLoggedIn(SPCR<gamestate::IPlayer> player)
        {
            gamestate::IWorld *world = m_world_tracker->FindWorld(player->GetAuthInfo().m_world_id);

            auto e = world->FindEntity(player->GetAuthInfo().m_entity_id);
            player->SetEntity(e);

        }

    private:
        SP<Hypodermic::Container> m_container;
        SP<gamestate::IWorldTracker> m_world_tracker;
        SP<gamestate::IRequestDispatcher> m_req;
        SP<pq::database> m_db;
        PlayerQueries m_queries;
    };
}


#include <Hypodermic/ContainerBuilder.h>

#pragma configurable ConfigureIPlayerSubsystem

namespace configure
{
    void ConfigureIPlayerSubsystem(Hypodermic::ContainerBuilder &container);
    void ConfigureIPlayerSubsystem(Hypodermic::ContainerBuilder &container)
    {
        container
            .registerType<server::PlayerSubsystem>()
            .as<server::IPlayerSubsystem>()
            .as<server::ISubsystem>()
            .singleInstance()
            .onActivated([](auto, auto inst) { inst->InitDB(); });
    }
}
