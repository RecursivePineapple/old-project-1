
#pragma target server

#include <mutex>
#include <map>
#include <Hypodermic/Container.h>

#include "Utils/dbwrapper/libpq.hpp"

#include "Common/Types.hpp"

#include "Interface/Game/IWorld.hpp"
#include "Interface/Component/IWorldTracker.hpp"

namespace gamestate
{
    namespace pq = dbwrapper::libpq;
    using dt = pq::basic_dtype;

    namespace queries
    {
        using namespace pq;

        using FindWorldType = SP<pq::prepared_query2<
            basic_dtype::UUID
        >>;

        using AddWorldType = SP<pq::prepared_query2<
            basic_dtype::UUID,
            basic_dtype::VARCHAR
        >>;

        static FindWorldType FindWorld(SPCR<database> db)
        {
            return db->prepare2<basic_dtype::UUID>(R"(
                SELECT
                    type
                FROM world
                WHERE world_id = $1::uuid
                  AND deleted = false
            )", "FindWorld");
        }

        static AddWorldType AddWorld(SPCR<database> db)
        {
            return db->prepare2<basic_dtype::UUID, basic_dtype::VARCHAR>(R"(
                INSERT INTO world (id, type) VALUES ($1::uuid, $2::varchar);
            )", "AddWorld");
        }
    }

    struct WorldTracker : public IWorldTracker
    {
        WorldTracker(
            SPCR<Hypodermic::Container> container,
            SPCR<pq::database> db
        ) : m_container(container),
            m_db(db)
            { }

        void InitDB()
        {
            m_find_world = queries::FindWorld(m_db);
            m_add_world = queries::AddWorld(m_db);
        }

        virtual IWorld* FindWorld(CR<uuid> id) override
        {
            std::lock_guard lock(m_world_mutex);

            auto iter = m_worlds.find(id);

            if(iter != m_worlds.end())
            {
                return iter->second.get();
            }

            auto res = m_find_world->exec(id);

            if(res.ntuples() == 1)
            {
                auto type = res.cell(0, 0).as_string().value();

                SP<IWorld> world = m_container->resolveNamed<IWorld>(type);
                world->SetId(id);
                world->Load();
                m_worlds[id] = world;

                return world.get();
            }
            
            return nullptr;
        }

        virtual IWorld* CreateHubWorld() override
        {
            return CreateWorld("HubWorld");
        }

        IWorld* CreateWorld(CR<std::string> type)
        {
            SP<IWorld> world = m_container->resolveNamed<IWorld>(type);
            world->SetId(uuid::generate());

            auto res = m_add_world->exec(world->GetId(), world->WorldType());

            world->Generate();
            world->Save();

            {
                std::lock_guard lock(m_world_mutex);

                m_worlds[world->GetId()] = world;
            }

            return world.get();
        }
    
        SP<Hypodermic::Container> m_container;
        SP<pq::database> m_db;

        queries::FindWorldType m_find_world;
        queries::AddWorldType m_add_world;

        std::mutex m_world_mutex;
        std::map<uuid, SP<IWorld>> m_worlds;
    };
}

#include <Hypodermic/ContainerBuilder.h>

namespace configure
{
    void ConfigureIWorldTracker(Hypodermic::ContainerBuilder &container);
    void ConfigureIWorldTracker(Hypodermic::ContainerBuilder &container)
    {
        container
            .registerType<gamestate::WorldTracker>()
            .as<gamestate::IWorldTracker>()
            .singleInstance()
            .onActivated([](auto, auto inst)
            {
                inst->InitDB();
            });
    }
}
