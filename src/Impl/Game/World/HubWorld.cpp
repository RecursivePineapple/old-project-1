
#pragma target server

#include <Hypodermic/Container.h>

#include "Utils/dbwrapper/libpq.hpp"
#include "Utils/queryhelper.hpp"

#include "Common/Types.hpp"

#include "Interface/Component/IUpdateTransmitter.hpp"

#include "Interface/Game/IPlayer.hpp"
#include "Interface/Game/IWorld.hpp"

#include "Impl/Game/World/EntityTracker.hpp"

/* #region Queries */

#define FIND_ALL_ENTITIES_PARAMS(X) X(UUID, world_id)

#define FIND_ALL_ENTITIES_QUERY(param)                                         \
  pq::table tbl("entity");                                                     \
  std::string sql = tbl.select(tbl.col("id"))                                  \
                        .where(tbl.col("world_id") == param(world_id) &&       \
                               tbl.col("deleted") == false);

#define FIND_ENTITY_PARAMS(X)                                                  \
  X(UUID, world_id)                                                            \
  X(UUID, entity_id)

#define FIND_ENTITY_QUERY(param)                                               \
  pq::table tbl("entity");                                                     \
  std::string sql =                                                            \
      tbl.select(tbl.col("x"), tbl.col("y"), tbl.col("z"), tbl.col("type"),    \
                 pq::functions::coalesce(tbl.col("data"), ""))                 \
          .where(tbl.col("world_id") == param(world_id) &&                     \
                 tbl.col("id") == param(entity_id) &&                          \
                 tbl.col("deleted") == false);

#define ADD_ENTITY_PARAMS(X)                                                   \
  X(UUID, world_id)                                                            \
  X(UUID, entity_id)                                                           \
  X(VARCHAR, type)                                                             \
  X(VARCHAR, data)                                                             \
  X(INT4, x)                                                                   \
  X(INT4, y)                                                                   \
  X(INT4, z)                                                                   \
  X(INT4, rx)                                                                  \
  X(INT4, ry)                                                                  \
  X(INT4, rz)                                                                  \
  X(INT4, sx)                                                                  \
  X(INT4, sy)                                                                  \
  X(INT4, sz)

#define ADD_ENTITY_QUERY(param)                                                \
  std::string sql = R"( \
        INSERT INTO entity (world_id, id, type, data, x, y, z, rx, ry, rz, sx, sy, sz) \
        VALUES($1::uuid, $2::uuid, $3::varchar, $4::json, \
            $5::integer, $6::integer, $7::integer, \
            $8::integer, $9::integer, $10::integer, \
            $11::integer, $12::integer, $13::integer) \
    )";

#define SAVE_ENTITY_PARAMS(X)                                                  \
  X(UUID, world_id)                                                            \
  X(UUID, entity_id)                                                           \
  X(VARCHAR, data)                                                             \
  X(INT4, x)                                                                   \
  X(INT4, y)                                                                   \
  X(INT4, z)                                                                   \
  X(INT4, rx)                                                                  \
  X(INT4, ry)                                                                  \
  X(INT4, rz)                                                                  \
  X(INT4, sx)                                                                  \
  X(INT4, sy)                                                                  \
  X(INT4, sz)

#define SAVE_ENTITY_QUERY(param)                                               \
  std::string sql = R"( \
        UPDATE entity SET \
            data = $3::json \
            x    = $4::integer, \
            y    = $5::integer, \
            z    = $6::integer, \
            rx   = $7::integer, \
            ry   = $8::integer, \
            rz   = $9::integer, \
            sx   = $10::integer, \
            sy   = $11::integer, \
            sz   = $12::integer, \
        WHERE world_id = $1::uuid \
            AND id = $2::uuid \
            AND deleted = false \
    )";

#define DELETE_ENTITY_PARAMS(X)                                                \
  X(UUID, world_id)                                                            \
  X(UUID, entity_id)

#define DELETE_ENTITY_QUERY(param)                                             \
  std::string sql = "UPDATE entity SET deleted = true WHERE world_id = "       \
                    "$1::uuid AND id = $2::uuid";

#define QUERIES_X(X)                                                           \
  X(FindAllEntities, FIND_ALL_ENTITIES_PARAMS, FIND_ALL_ENTITIES_QUERY)        \
  X(FindEntity, FIND_ENTITY_PARAMS, FIND_ENTITY_QUERY)                         \
  X(AddEntity, ADD_ENTITY_PARAMS, ADD_ENTITY_QUERY)                            \
  X(SaveEntity, SAVE_ENTITY_PARAMS, SAVE_ENTITY_QUERY)                         \
  X(DeleteEntity, DELETE_ENTITY_PARAMS, DELETE_ENTITY_QUERY)

/* #endregion */

namespace gamestate {
    namespace pq = dbwrapper::libpq;
    QUERY_STRUCT_DECL(HubQueries, QUERIES_X)

    struct HubWorld : public IWorld {
        HubWorld(
            SPCR<Hypodermic::Container> container,
            SPCR<dbwrapper::libpq::database> db,
            SPCR<IUpdateTransmitter> transmitter
        ) : m_container(container),
            m_db(db),
            m_transmitter(transmitter)
        {
            m_tracker.OnEntityAdded().Bind<&HubWorld::OnEntityAdded>(this);
            m_tracker.OnEntityRemoved().Bind<&HubWorld::OnEntityRemoved>(this);
        }

        void InitDB() { m_queries = HubQueries(m_db); }

        virtual std::string WorldType() override { return "HubWorld"; }

        virtual SP<INamedEntity> FindEntity(CR<uuid> id) override
        {
            auto e = m_tracker.FindEntity(id);

            if (e != nullptr)
            {
                return e;
            }

            return LoadEntity(id);
        }

        virtual SP<INamedEntity> Spawn(CR<std::string> type, CR<INamedEntity::TransformType> loc) override
        {
            SP<INamedEntity> e = m_container->resolveNamed<INamedEntity>(type);
            e->SetId(uuid::generate());

            e->SetLocation(INamedEntity::TransformType(this, loc.m_transform.m_loc));

            if(e->IsActive())
            {
                AddEntity(e);
                e->BeginPlay();
            }

            return e;
        }

        virtual void Remove(SPCR<INamedEntity> entity) override
        {
            entity->EndPlay();

            std::unique_lock lock(m_entity_mutex);
            m_tracker.Remove(entity);
        }

        virtual void Destroy(SPCR<INamedEntity> entity) override
        {
            Remove(entity);
            m_queries.DeleteEntity(GetId(), entity->GetId());
        }

        virtual void AddPlayer(SPCR<IPlayer> player) override
        {
            {
                std::unique_lock lock(m_player_mutex);
                m_players[player->GetEntity()->GetId()] = player;
            }

            {
                std::shared_lock lock(m_entity_mutex);
                for(auto const& e : m_tracker.m_entity_list)
                {
                    m_transmitter->SendCreate(this, player, e);
                }
            }
        }

        virtual void RemovePlayer(SPCR<IPlayer> player) override
        {
            std::unique_lock lock(m_player_mutex);

            m_players.erase(player->GetEntity()->GetId());
        }

        virtual void Dispatch(ConnectionContext *sender, CR<server::Message> msg) override
        {
            std::shared_lock lock(m_entity_mutex);

            m_tracker.Dispatch(sender, msg);
        }

        virtual void Load() override
        {
            std::unique_lock lock(m_entity_mutex);

            auto e = m_queries.FindAllEntities(GetId());

            for (int i = 0; i < e.ntuples(); ++i)
            {
                auto id = e.cell(i, 0).as<uuid>().value();

                LoadEntity(uuid(id.value));
            }
        }

        virtual void Save() override
        {
            std::shared_lock lock(m_entity_mutex);

            for (auto const &e : m_tracker.m_entity_list)
            {
                SaveEntity(e);
            }
        }

        virtual void Generate() override
        {
            std::unique_lock lock(m_entity_mutex);
        }

        SP<INamedEntity> LoadEntity(CR<uuid> id)
        {
            auto res = m_queries.FindEntity(GetId(), id.value);

            if (res.ntuples() != 1)
            {
                return nullptr;
            }

            auto t = pq::to_nonoptional_tuple<int, int, int, std::string, std::string>(res, 0);

            if (!t)
            {
                return nullptr;
            }

            auto [x, y, z, type, data] = t.value();

            SP<INamedEntity> e = m_container->resolveNamed<INamedEntity>(type);

            if (e == nullptr)
            {
                return nullptr;
            }

            auto state = jsontypes::span_t::FromString(data);

            if(!state)
            {
                return nullptr;
            }

            e->SetId(id);
            e->SetLocation(INamedEntity::TransformType(this, x, y, z));
            e->LoadState(state.value());

            if(e->IsActive())
            {
                {
                    std::unique_lock lock(m_entity_mutex);
                    m_tracker.Add(e);
                }

                e->BeginPlay();
            }

            return e;
        }

        void AddEntity(SPCR<INamedEntity> e)
        {
            {
                std::unique_lock lock(m_entity_mutex);
                m_tracker.Add(e);
            }

            if(!e->ShouldSave()) { return; }

            auto t = e->GetLocation().m_transform;

            m_queries.AddEntity(
                GetId(), e->GetId(),
                e->EntityType(),
                e->SaveState(),
                t.m_loc.x, t.m_loc.y, t.m_loc.z,
                t.m_rot.x, t.m_rot.y, t.m_rot.z,
                t.m_scale.x, t.m_scale.y, t.m_scale.z);
        }

        void SaveEntity(SPCR<INamedEntity> e)
        {
            if(!e->ShouldSave()) { return; }

            auto t = e->GetLocation().m_transform;

            m_queries.SaveEntity(
                GetId(), e->GetId(),
                e->SaveState(),
                t.m_loc.x, t.m_loc.y, t.m_loc.z,
                t.m_rot.x, t.m_rot.y, t.m_rot.z,
                t.m_scale.x, t.m_scale.y, t.m_scale.z);
        }

        void OnEntityAdded(INamedEntity *entity)
        {
            entity->OnStateChanged().Bind<&HubWorld::OnEntityStateChanged>(this);
            entity->OnMove().Bind<&HubWorld::OnEntityMoved>(this);

            std::shared_lock lock(m_player_mutex);

            for(auto const& player : m_players)
            {
                m_transmitter->SendCreate(this, player.second, entity);
            }
        }

        void OnEntityRemoved(INamedEntity *entity)
        {
            std::shared_lock lock(m_player_mutex);

            for(auto const& player : m_players)
            {
                m_transmitter->SendDestroy(this, player.second, entity);
            }
        }

        void OnEntityStateChanged(INamedEntity *entity)
        {
            std::shared_lock lock(m_player_mutex);

            for(auto const& player : m_players)
            {
                m_transmitter->SendUpdate(this, player.second, entity);
            }
        }

        void OnEntityMoved(INamedEntity *entity)
        {
            std::shared_lock lock(m_player_mutex);

            for(auto const& player : m_players)
            {
                m_transmitter->SendUpdatePhysics(this, player.second, entity);
            }
        }

        SP<Hypodermic::Container> m_container;
        SP<dbwrapper::libpq::database> m_db;
        HubQueries m_queries;
        SP<IUpdateTransmitter> m_transmitter;

        std::shared_mutex m_entity_mutex, m_player_mutex;
        EntityTracker m_tracker;
        std::map<uuid, SP<IPlayer>> m_players;
    };
}

#include <Hypodermic/ContainerBuilder.h>

#pragma configurable ConfigureHubWorld

namespace configure {
    void ConfigureHubWorld(Hypodermic::ContainerBuilder &container);
    void ConfigureHubWorld(Hypodermic::ContainerBuilder &container) {
        container.registerType<gamestate::HubWorld>()
            .named<gamestate::IWorld>("HubWorld")
            .onActivated([](auto, auto inst) { inst->InitDB(); });
    }
}
