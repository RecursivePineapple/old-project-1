
#pragma target server

#include <Hypodermic/Container.h>

#include "Utils/dbwrapper/libpq.hpp"

#include "Common/Types.hpp"

#include "Interface/Game/IWorld.hpp"

#include "Impl/Game/World/EntityTracker.hpp"

#include "HubSession.hpp"

namespace gamestate
{
    namespace pq = dbwrapper::libpq;

    namespace queries
    {
        using namespace pq;

        using FindAllEntitiesType = SP<pq::prepared_query2<
            basic_dtype::UUID
        >>;

        using FindEntityType = SP<pq::prepared_query2<
            basic_dtype::UUID,
            basic_dtype::UUID
        >>;

        using SaveEntityType = SP<pq::prepared_query2<
            basic_dtype::UUID,
            basic_dtype::UUID,
            basic_dtype::INT4,
            basic_dtype::INT4,
            basic_dtype::INT4,
            basic_dtype::VARCHAR
        >>;

        using DeleteEntityType = SP<pq::prepared_query2<
            basic_dtype::UUID
        >>;

        static FindAllEntitiesType FindAllEntities(SPCR<database> db)
        {
            return db->prepare2<basic_dtype::UUID>(R"(
                SELECT
                    id
                FROM entity
                WHERE world_id = $1::uuid
                  AND deleted = false
            )", db->get_prepared_query_name());
        }

        static FindEntityType FindEntity(SPCR<database> db)
        {
            return db->prepare2<basic_dtype::UUID, basic_dtype::UUID>(R"(
                SELECT
                    x, y, z, type, data
                FROM entity
                WHERE world_id = $1::uuid
                  AND id = $2::uuid
                  AND deleted = false
            )", db->get_prepared_query_name());
        }

        static SaveEntityType SaveEntity(SPCR<database> db)
        {
            return db->prepare2<
                    basic_dtype::UUID,
                    basic_dtype::UUID,
                    basic_dtype::INT4,
                    basic_dtype::INT4,
                    basic_dtype::INT4,
                    basic_dtype::VARCHAR>(R"(
                UPDATE entity SET
                    x = $3::integer,
                    y = $4::integer,
                    z = $5::integer,
                    data = $6::json
                WHERE world_id = $1::uuid
                  AND id = $2::uuid
                  AND deleted = false
            )", db->get_prepared_query_name());
        }

        static DeleteEntityType DeleteEntity(SPCR<database> db)
        {
            return db->prepare2<basic_dtype::UUID>(R"(
                UPDATE entity SET deleted = true WHERE id = $1::uuid
            )", db->get_prepared_query_name());
        }
    }

    struct HubWorld : public IWorld
    {
        HubWorld(
            SP<Hypodermic::Container> container,
            SPCR<dbwrapper::libpq::database> db
        ) : m_container(container),
            m_db(db),
            m_session(container->resolveNamed<ISession>("HubSession"))
            { }
        
        void InitDB()
        {
            m_find_all_entities = queries::FindAllEntities(m_db);
            m_find_entity = queries::FindEntity(m_db);
            m_save_entity = queries::SaveEntity(m_db);
            m_delete_entity = queries::DeleteEntity(m_db);
        }

        virtual std::string WorldType() override { return "HubWorld"; }

        virtual EntityLockResult TryLockEntitiesWithin(CR<Location>, long, ISession* locker) override
        {
            if(locker != m_session.get())
            {
                return EntityLockResult({m_session.get()}, {}, {});
            }

            std::map<uuid, SP<INamedEntity>> entities;
            std::unordered_set<uuid> ids;

            for(auto const& e : m_tracker.m_entity_list)
            {
                entities[e->GetId()] = e;
                ids.insert(e->GetId());
            }

            return EntityLockResult({}, std::move(entities), std::move(ids));
        }

        virtual SP<INamedEntity> FindEntity(CR<uuid> id) override
        {
            std::shared_lock lock(m_entity_mutex);

            auto e = m_tracker.FindEntity(id);

            if(e != nullptr)
            {
                return e;
            }

            return LoadEntity(id);
        }

        virtual SP<INamedEntity> Spawn(
            CR<std::string> type,
            CR<INamedEntity::LocationType> loc,
            ISession* locker = nullptr) override
        {
            SP<INamedEntity> e = m_container->resolveNamed<INamedEntity>(type);

            std::unique_lock lock(m_entity_mutex);

            m_tracker.Add(e);

            INamedEntity::LocationType l(loc);
            l.m_world = this;
            e->SetLocation(l);
            
            if(locker != nullptr)
            {
                e->TryLock(locker);
            }

            e->BeginPlay();

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
            entity->EndPlay();

            std::unique_lock lock(m_entity_mutex);

            m_delete_entity->exec(entity->GetId());

            m_tracker.Remove(entity);
        }

        virtual void Dispatch(CR<server::Message> msg) override
        {
            std::shared_lock lock(m_entity_mutex);

            m_tracker.Dispatch(msg);
        }
        
        virtual void OnEntityMoved(CR<uuid>, CR<Location>) override { }

        virtual ISession* FindSessionAt(CR<Location>) override
        {
            return m_session.get();
        }

        virtual void Load() override
        {
            std::unique_lock lock(m_entity_mutex);

            auto e = m_find_all_entities->exec(m_world_id);

            for(int i = 0; i < e.ntuples(); ++i)
            {
                auto id = e.cell(i, 0).as<uuid>().value();

                LoadEntity(uuid(id.value));
            }

            dynamic_cast<IHubSession*>(m_session.get())->SetLockedEntities(m_tracker.m_entity_list);
        }

        virtual void Save() override
        {
            std::shared_lock lock(m_entity_mutex);

            for(auto const& e : m_tracker.m_entity_list)
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
            auto res = m_find_entity->exec(m_world_id, id.value);

            if(res.ntuples() != 1)
            {
                return nullptr;
            }

            int x = res.cell(0, 0).as_int().value();
            int y = res.cell(0, 1).as_int().value();
            int z = res.cell(0, 2).as_int().value();

            std::string type = res.cell(0, 3).as_string().value();
            auto data = res.cell(0, 4).as_string();

            SP<INamedEntity> e = m_container->resolveNamed<INamedEntity>(type);

            if(e == nullptr)
            {
                return nullptr;
            }

            e->SetId(id);
            e->SetLocation(INamedEntity::LocationType(this, x, y, z));

            if(data)
            {
                e->LoadState(data.value());
            }

            m_tracker.Add(e);

            e->BeginPlay();

            return e;
        }

        void SaveEntity(SPCR<INamedEntity> e)
        {
            auto l = e->GetLocation().m_loc;

            m_save_entity->exec(
                m_world_id, e->GetId(),
                l.comp.x, l.comp.y, l.comp.z,
                e->SaveState()
            );
        }

        SP<Hypodermic::Container> m_container;
        SP<dbwrapper::libpq::database> m_db;

        queries::FindAllEntitiesType m_find_all_entities;
        queries::FindEntityType m_find_entity;
        queries::SaveEntityType m_save_entity;
        queries::DeleteEntityType m_delete_entity;

        std::shared_mutex m_entity_mutex;
        uuid m_world_id;
        EntityTracker m_tracker;
        SP<ISession> m_session;
    };
}

#include <Hypodermic/ContainerBuilder.h>

namespace configure
{
    void ConfigureHubWorld(Hypodermic::ContainerBuilder &container);
    void ConfigureHubWorld(Hypodermic::ContainerBuilder &container)
    {
        container
            .registerType<gamestate::HubWorld>()
            .named<gamestate::IWorld>("HubWorld")
            .onActivated([](auto, auto inst)
            {
                inst->InitDB();
            });
    }
}
