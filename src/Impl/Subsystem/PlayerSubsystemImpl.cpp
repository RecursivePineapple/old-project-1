
#pragma target server

#include <Hypodermic/Container.h>
#include <spdlog/spdlog.h>

#include "Utils/dbwrapper/libpq.hpp"
#include "Utils/queryhelper.hpp"

#include "Common/Types.hpp"
#include "Common/PlayerAuthInfo.hpp"

#include "Interface/Subsystem/IPlayerSubsystem.hpp"
#include "Interface/Component/IWorldTracker.hpp"

/* #region Queries */

#define FIND_PLAYER_PARAMS(X) X(VARCHAR, username)

#define FIND_PLAYER_QUERY(param) \
    pq::table tbl("player"); \
    std::string sql = tbl.select( \
        tbl.col("secret"), tbl.col("world_id"), tbl.col("entity_id") \
    ).where( \
        tbl.col("username") == param(username) && \
        tbl.col("deleted") == false \
    );

#define QUERIES_X(X) \
  X(FindPlayer, FIND_PLAYER_PARAMS, FIND_PLAYER_QUERY)

/* #endregion */

namespace server
{
    #define PLAYER_AUTH_FIELDS(X) \
        X(std::string, jsontypes::string, username) \
        X(std::string, jsontypes::string, secret)

    DECLARE_JSON_STRUCT(PlayerAuthMsg, PLAYER_AUTH_FIELDS)

    namespace pq = dbwrapper::libpq;
    QUERY_STRUCT_DECL(PlayerQueries, QUERIES_X)

    struct PlayerSubsystem : public IPlayerSubsystem
    {
        PlayerSubsystem(
            SPCR<Hypodermic::Container> container,
            SPCR<gamestate::IWorldTracker> world_tracker,
            SPCR<pq::database> db
        ) : m_container(container),
            m_world_tracker(world_tracker),
            m_db(db)
            { }
        
        void InitDB() { m_queries = PlayerQueries(m_db); }

        virtual SP<gamestate::IPlayer> CreateUnathenticated() override
        {
            return m_container->resolve<gamestate::IPlayer>();
        }

        virtual void Disconnect(SPCR<gamestate::IPlayer>) override
        {

        }

        virtual void OnMessage(ConnectionContext *sender, CR<server::Message> msg) override
        {
            if(msg.action == "set-auth-token")
            {
                PlayerAuthMsg auth;

                auto player = sender->m_player;
                auto conn = player->GetConnection();

                if(!msg.data || !msg.data->ParseInto<PlayerAuthMsg>(auth) || auth.username.empty())
                {
                    conn->Send(MessageBuilder(MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION).action("reject-auth").data({
                        {"message", "invalid message format"}
                    }).build());

                    spdlog::debug("player auth message handled but not valid");
                    return;
                }

                auto res = m_queries.FindPlayer(auth.username);

                if(res.ntuples() != 1)
                {
                    conn->Send(MessageBuilder(MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION).action("reject-auth").data({
                        {"message", "invalid token"}
                    }).build());

                    spdlog::debug("auth attempt for user {0} had invalid username", auth.username);
                    return;
                }

                auto tpl = pq::to_nonoptional_tuple<std::string, uuid, uuid>(res, 0);

                if(!tpl)
                {
                    conn->Send(MessageBuilder(MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION).action("reject-auth").data({
                        {"message", "invalid username or secret"}
                    }).build());
                    spdlog::debug("auth attempt for user {0} had invalid username", auth.username);
                    return;
                }

                auto [secret, world_id, entity_id] = tpl.value();

                if(secret != auth.secret)
                {
                    conn->Send(MessageBuilder(MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION).action("reject-auth").data({
                        {"message", "invalid username or secret"}
                    }).build());
                    spdlog::debug("auth attempt for user {0} had invalid secret", auth.username);
                    return;
                }

                player->SetAuthInfo(gamestate::PlayerAuthInfo(auth.username, world_id, entity_id));

                conn->Send(MessageBuilder(MessageType::MESSAGE_TYPE_SUBSYSTEM_ACTION).action("accept-auth").build());
                spdlog::debug("player authenticated as {0} with world_id {1} and entity_id {2}",
                    auth.username,
                    world_id.to_string(),
                    entity_id.to_string());

                OnPlayerLoggedIn(player);
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
        SP<pq::database> m_db;
        PlayerQueries m_queries;
    };
}


#include <Hypodermic/ContainerBuilder.h>

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
