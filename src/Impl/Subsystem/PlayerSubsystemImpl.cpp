
#pragma target server

#include <Hypodermic/Container.h>
#include <spdlog/spdlog.h>

#include "Utils/dbwrapper/libpq.hpp"

#include "Common/Types.hpp"
#include "Common/PlayerAuthInfo.hpp"

#include "Interface/Subsystem/IPlayerSubsystem.hpp"
#include "Interface/Component/IWorldTracker.hpp"

namespace server
{
    #define PLAYER_AUTH_FIELDS(X) \
        X(std::string, jsontypes::string, username) \
        X(std::string, jsontypes::string, secret)

    DECLARE_JSON_STRUCT(PlayerAuthMsg, PLAYER_AUTH_FIELDS)

    namespace pq = dbwrapper::libpq;

    namespace queries
    {
        using namespace pq;

        using FindPlayerType = SP<pq::prepared_query2<basic_dtype::VARCHAR>>;

        static FindPlayerType FindPlayer(SPCR<database> db)
        {
            return db->prepare2<basic_dtype::VARCHAR>(R"(
                SELECT
                    secret, world_id, entity_id
                FROM player
                WHERE username = $1::varchar
                  AND deleted = false
            )", db->get_prepared_query_name());
        }
    }

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
        
        void InitDB()
        {
            m_find_player = queries::FindPlayer(m_db);
        }

        virtual SP<gamestate::IPlayer> CreateUnathenticated() override
        {
            return m_container->resolve<gamestate::IPlayer>();
        }

        virtual void Disconnect(SPCR<gamestate::IPlayer>) override
        {

        }

        virtual void OnMessage(CR<server::Message> msg) override
        {
            if(msg.Type() == "auth")
            {
                PlayerAuthMsg auth;

                if(!msg.ParseInto(auth) || auth.username.empty())
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "error"},
                        {"message", "invalid message format"}
                    });
                    spdlog::debug("player auth message handled but not valid");
                    return;
                }

                auto res = m_find_player->exec(auth.username);

                if(res.ntuples() != 1)
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "error"},
                        {"message", "invalid username or secret"}
                    });
                    spdlog::debug("auth attempt for user {0} had invalid username", auth.username);
                    return;
                }

                auto [secret, world_id, entity_id] = pq::to_tuple<std::string, uuid, uuid>(res, 0);

                if(secret != auth.secret)
                {
                    msg.Sender()->m_player->GetConnection()->SendObject({
                        {"status", "error"},
                        {"message", "invalid username or secret"}
                    });
                    spdlog::debug("auth attempt for user {0} had invalid secret", auth.username);
                    return;
                }

                gamestate::PlayerAuthInfo authinfo;
                authinfo.is_authenticated = true;
                authinfo.username = auth.username;
                authinfo.world_id = uuid(world_id.value().value);
                authinfo.entity_id = uuid(entity_id.value().value);

                msg.Sender()->m_player->SetAuthInfo(authinfo);

                msg.Sender()->m_player->GetConnection()->SendObject({
                    {"status", "success"}
                });
                spdlog::debug("player authenticated as {0} with world_id {1} and entity_id {2}",
                    authinfo.username,
                    authinfo.world_id.to_string(),
                    authinfo.entity_id.to_string());

                OnPlayerLoggedIn(msg.Sender()->m_player);
            }
        }

        void OnPlayerLoggedIn(SPCR<gamestate::IPlayer> player)
        {
            gamestate::IWorld *world = m_world_tracker->FindWorld(player->GetAuthInfo().world_id);

            auto e = world->FindEntity(player->GetAuthInfo().entity_id);
            player->SetEntity(e);

            world->FindSessionAt(e->GetLocation().m_loc)->AddPlayer(player);
        }

    private:
        SP<Hypodermic::Container> m_container;
        SP<gamestate::IWorldTracker> m_world_tracker;
        SP<pq::database> m_db;
        queries::FindPlayerType m_find_player;
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
            .onActivated([](auto, auto inst)
            {
                inst->InitDB();
            });
    }
}
