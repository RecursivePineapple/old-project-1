
#pragma target server

#include <mutex>
#include <map>
#include <Hypodermic/Container.h>

#include "Utils/dbwrapper/libpq.hpp"
#include "Utils/queryhelper.hpp"

#include "Common/Types.hpp"

#include "Interface/Game/IWorld.hpp"
#include "Interface/Component/IWorldTracker.hpp"

#define FIND_WORLD_PARAMS(X) \
    X(UUID, world_id)

#define FIND_WORLD_QUERY(param) \
    pq::table tbl("world"); \
    std::string sql = tbl.select( \
        tbl.col("type") \
    ) \
    .where( \
        tbl.col("world_id") == param(world_id) && \
        tbl.col("deleted") == false \
    );

#define ADD_WORLD_PARAMS(X) \
    X(UUID, world_id) \
    X(VARCHAR, type)

#define ADD_WORLD_QUERY(param) \
    pq::table tbl("world"); \
    std::string sql = "INSERT INTO world (world_id, type) VALUES ($1::uuid, $2::varchar);";

#define QUERIES_X(X) \
    X(FindWorld, FIND_WORLD_PARAMS, FIND_WORLD_QUERY) \
    X(AddWorld, ADD_WORLD_PARAMS, ADD_WORLD_QUERY)

namespace gamestate
{
    namespace pq = dbwrapper::libpq;

    QUERY_STRUCT_DECL(TrackerQuery, QUERIES_X)
 
    struct WorldTracker : public IWorldTracker
    {
        WorldTracker(
            SPCR<Hypodermic::Container> container,
            SPCR<pq::database> db
        ) : m_container(container),
            m_db(db)
            { }

        virtual IWorld* FindLoadedWorld(CR<uuid> id) override
        {
            std::lock_guard lock(m_world_mutex);

            auto iter = m_worlds.find(id);

            if(iter != m_worlds.end())
            {
                return iter->second.get();
            }
            else
            {
                return nullptr;
            }
        }

        virtual IWorld* FindWorld(CR<uuid> id) override
        {
            std::lock_guard lock(m_world_mutex);

            IWorld *loaded = FindLoadedWorld(id);

            if(loaded) { return loaded; }

            auto res = m_q.FindWorld(id);

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

            auto res = m_q.AddWorld(world->GetId(), world->WorldType());

            world->Generate();

            if(!world->IsTransient())
            {
                world->Save();
            }

            {
                std::lock_guard lock(m_world_mutex);

                m_worlds[world->GetId()] = world;
            }

            return world.get();
        }
    
        void InitDB() { m_q = TrackerQuery(m_db); }
        
        SP<Hypodermic::Container> m_container;
        SP<pq::database> m_db;
        TrackerQuery m_q;

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
            .onActivated([](auto, auto inst) { inst->InitDB(); });
    }
}
