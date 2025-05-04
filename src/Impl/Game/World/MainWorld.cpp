
// #pragma target server

#include <Hypodermic/Container.h>

#include "Utils/dbwrapper/libpq.hpp"

#include "Common/Types.hpp"

#include "Interface/Game/IWorld.hpp"

#include "Impl/Game/World/EntityTracker.hpp"

namespace gamestate
{
    namespace pq = dbwrapper::libpq;

    namespace queries
    {
        using namespace pq;

        using FindEntitiesWithinType = SP<pq::prepared_query2<
            basic_dtype::INT4,
            basic_dtype::INT4,
            basic_dtype::INT4,
            basic_dtype::INT4
        >>;

        using FindEntityType = SP<pq::prepared_query2<
            basic_dtype::INT4,
            basic_dtype::UUID
        >>;

        using SaveEntityType = SP<pq::prepared_query2<
            basic_dtype::INT4,
            basic_dtype::UUID,
            basic_dtype::INT4,
            basic_dtype::INT4,
            basic_dtype::INT4,
            basic_dtype::VARCHAR
        >>;

        using DeleteEntityType = SP<pq::prepared_query2<
            basic_dtype::UUID
        >>;

        static FindEntitiesWithinType FindEntitiesWithin(SPCR<database> db)
        {
            return db->prepare2<basic_dtype::INT4, basic_dtype::INT4, basic_dtype::INT4, basic_dtype::INT4>(R"(
                SELECT
                    id
                FROM entity
                WHERE ( POW(x - $1::int4, 2) + POW(y - $2::int4, 2) + POW(z - $3::int4, 2) ) < POW($4::int4, 2)
                  AND deleted = false
            )", db->get_prepared_query_name());
        }

        static FindEntityType FindEntity(SPCR<database> db)
        {
            return db->prepare2<basic_dtype::INT4, basic_dtype::UUID>(R"(
                SELECT
                    x, y, z, type, data
                FROM entity
                WHERE world_id = $1::integer
                  AND id = $2::uuid
                  AND deleted = false
            )", db->get_prepared_query_name());
        }

        static SaveEntityType SaveEntity(SPCR<database> db)
        {
            return db->prepare2<
                    basic_dtype::INT4,
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
                WHERE world_id = $1::integer
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

    struct MainWorld : public IWorld
    {
        MainWorld(
            SP<Hypodermic::Container> container,
            SPCR<dbwrapper::libpq::database> db
        ) : m_container(container),
            m_db(db),
            m_find_entities(queries::FindEntitiesWithin(db)),
            m_find_entity(queries::FindEntity(db)),
            m_delete_entity(queries::DeleteEntity(db)) { }
        
        virtual std::string WorldType() override { return "MainWorld"; }

        virtual SP<INamedEntity> Spawn(
            CR<std::string> type,
            CR<INamedEntity::LocationType> loc) override
        { 
            SP<INamedEntity> e = m_container->resolveNamed<INamedEntity>(type);

            m_tracker.Add(e);
            m_grid.Add(e);

            INamedEntity::LocationType l(loc);
            l.m_world = this;
            e->SetLocation(l);
            
            e->BeginPlay();

            return e;
        }

        virtual void Remove(SPCR<INamedEntity> entity) override
        {
            m_tracker.Remove(entity);
            m_grid.Remove(entity);
        }

        virtual void Destroy(SPCR<INamedEntity> entity) override
        {
            m_delete_entity->exec(entity->GetId());

            m_tracker.Remove(entity);
            m_grid.Remove(entity);
        }

        virtual void Dispatch(CR<server::Message> msg) override
        {
            m_tracker.Dispatch(msg);
        }

        virtual void Load() override
        {

        }

        virtual void Save() override
        {
            
        }

        SP<INamedEntity> LoadEntity(CR<uuid> id)
        {
            auto res = m_find_entity->exec(m_world_id, id);

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
            m_grid.Add(e);

            e->BeginPlay();

            return e;
        }

        SP<Hypodermic::Container> m_container;
        SP<dbwrapper::libpq::database> m_db;

        int m_world_id;
        EntityTracker m_tracker;
        EntityGrid m_grid;

        queries::FindEntitiesWithinType m_find_entities;
        queries::FindEntityType m_find_entity;
        queries::DeleteEntityType m_delete_entity;
    };
}

#include "Impl/ConfigureImpl.hpp"

// #pragma configurable ConfigureMainWorld

namespace configure
{
    void ConfigureMainWorld(Hypodermic::ContainerBuilder &container)
    {
        container
            .registerType<gamestate::MainWorld>()
            .as<gamestate::IWorld>()
            .named("MainWorld");
    }
}
