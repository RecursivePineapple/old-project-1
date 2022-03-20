
#pragma once

#include <mutex>
#include <shared_mutex>
#include <map>
#include <vector>
#include <unordered_set>
#include <spdlog/spdlog.h>

#include "Utils/uuid.hpp"

#include "Common/Types.hpp"
#include "Common/Transform.hpp"
#include "Common/Message.hpp"
#include "Common/Event.hpp"

#include "Interface/Game/INamedEntity.hpp"

namespace gamestate
{
    struct EntityTracker
    {
        SP<INamedEntity> FindEntity(CR<uuid> id)
        {
            std::shared_lock lock(m_entities_lock);

            auto iter = m_entities.find(id);

            if(iter != m_entities.end())
            {
                return iter->second;
            }

            return nullptr;
        }

        void FindEntities(CR<std::vector<uuid>> ids, std::vector<SP<INamedEntity>> &found, std::vector<uuid> &missing)
        {
            found.clear();
            missing.clear();

            std::shared_lock lock(m_entities_lock);

            for(auto const& id : ids)
            {
                auto iter = m_entities.find(id);

                if(iter != m_entities.end())
                {
                    found.push_back(iter->second);
                }
                else
                {
                    missing.push_back(id);
                }
            }
        }

        template<typename iter_t>
        void Add(iter_t begin, iter_t end)
        {
            std::unique_lock lock(m_entities_lock);

            while(begin != end)
            {
                m_entities[begin->GetId()] = *begin;
                if(m_entity_list.insert(*begin).second)
                {
                    m_entity_added.Dispatch((*begin).get());
                }

                ++begin;
            }
        }

        void Add(SPCR<INamedEntity> entity)
        {
            std::unique_lock lock(m_entities_lock);

            m_entities[entity->GetId()] = entity;
            if(m_entity_list.insert(entity).second)
            {
                m_entity_added.Dispatch(entity.get());
            }
        }

        template<typename iter_t>
        void Remove(iter_t begin, iter_t end)
        {
            std::unique_lock lock(m_entities_lock);

            while(begin != end)
            {
                m_entities.erase(begin->GetId());
                if(m_entity_list.erase(*begin) > 0)
                {
                    m_entity_removed.Dispatch((*begin).get());
                }

                ++begin;
            }
        }

        void Remove(SPCR<INamedEntity> entity)
        {
            std::unique_lock lock(m_entities_lock);

            m_entities.erase(entity->GetId());
            if(m_entity_list.erase(entity) > 0)
            {
                m_entity_removed.Dispatch(entity.get());
            }
        }

        void Dispatch(ConnectionContext *sender, CR<server::Message> msg)
        {
            std::shared_lock lock(m_entities_lock);

            auto iter = m_entities.find(msg.id.value());

            if(iter == m_entities.end())
            {
                std::stringstream ss;
                server::Message::emit(ss, msg);
                spdlog::warn("received entity message for invalid entity: {0}", ss.str());
                return;
            }

            if(msg.action)
            {
                iter->second->OnMessage(sender, msg.action.value(), msg.data);
            }
        }

        Event<INamedEntity*> OnEntityAdded()
        {
            return m_entity_added;
        }

        Event<INamedEntity*> OnEntityRemoved()
        {
            return m_entity_removed;
        }

        std::shared_mutex m_entities_lock;
        std::map<uuid, SP<INamedEntity>> m_entities;
        std::unordered_set<SP<INamedEntity>> m_entity_list;

        Event<INamedEntity*> m_entity_added, m_entity_removed;
    };

    // struct EntityGrid
    // {
    //     const static long GridSize = 100;
    //     typedef std::tuple<long, long, long> GridCoordType;

    //     void FindEntitiesWithin(CR<Location> center, long radius, std::unordered_set<SP<INamedEntity>> &found)
    //     {
    //         std::shared_lock lock(m_grid_lock);

    //         radius /= GridSize;
    //         double r2 = radius*radius;

    //         for(long x = 0, x2 = 0; x2 <= r2; ++x, x2=x*x)
    //         {
    //             for(long y = 0, y2 = 0; (x2+y2) <= r2; ++y, y2=y*y)
    //             {
    //                 for(long z = 0, z2 = 0; (x2+y2+z2) <= r2; ++z, z2=z*z)
    //                 {
    //                     auto iter = m_grid.find(std::make_tuple(
    //                         x + center.comp.x,
    //                         y + center.comp.y,
    //                         z + center.comp.z
    //                     ));

    //                     if(iter != m_grid.end())
    //                     {
    //                         found.reserve(found.size() + iter->second.size());

    //                         found.insert(iter->second.begin(), iter->second.end());
    //                     }
    //                 }
    //             }
    //         }
    //     }

    //     void OnEntityMoved(EntityTracker &tracker, CR<uuid> id, CR<Location> old_loc)
    //     {
    //         std::shared_lock lock(m_grid_lock);

    //         auto e = tracker.FindEntity(id);

    //         if(e == nullptr)
    //         {
    //             return;
    //         }

    //         if(GridLoc(old_loc) == GridLoc(e))
    //         {
    //             return;
    //         }

    //         {
    //             std::unique_lock lock2(m_grid_lock);

    //             auto g_old = m_grid.find(GridLoc(old_loc));

    //             if(g_old != m_grid.end())
    //             {
    //                 g_old->second.erase(e);
    //             }

    //             auto g_new = m_grid.find(GridLoc(e));

    //             if(g_new == m_grid.end())
    //             {
    //                 m_grid.try_emplace(GridLoc(e), std::unordered_set<SP<INamedEntity>>{e});
    //             }
    //             else
    //             {
    //                 g_new->second.insert(e);
    //             }
    //         }
    //     }

    //     template<typename iter_t>
    //     void Add(iter_t begin, iter_t end)
    //     {
    //         std::unique_lock lock(m_grid_lock);

    //         while(begin != end)
    //         {
    //             auto loc = GridLoc(*begin);
    //             auto grid = m_grid.find(loc);

    //             if(grid != m_grid.end())
    //             {
    //                 grid->second.insert(*begin);
    //             }
    //             else
    //             {
    //                 m_grid.try_emplace(loc, std::unordered_set<SP<INamedEntity>>{
    //                     *begin
    //                 });
    //             }

    //             ++begin;
    //         }
    //     }

    //     void Add(SPCR<INamedEntity> entity)
    //     {
    //         std::unique_lock lock(m_grid_lock);

    //         auto loc = GridLoc(entity);
    //         auto grid = m_grid.find(loc);

    //         if(grid != m_grid.end())
    //         {
    //             grid->second.insert(entity);
    //         }
    //         else
    //         {
    //             m_grid.try_emplace(loc, std::unordered_set<SP<INamedEntity>>{
    //                 entity
    //             });
    //         }
    //     }

    //     template<typename iter_t>
    //     void Remove(iter_t begin, iter_t end)
    //     {
    //         std::unique_lock lock(m_grid_lock);

    //         while(begin != end)
    //         {
    //             auto grid = m_grid.find(GridLoc(*begin));

    //             if(grid != m_grid.end())
    //             {
    //                 grid->second.erase(*begin);
    //             }

    //             ++begin;
    //         }
    //     }

    //     void Remove(SPCR<INamedEntity> entity)
    //     {
    //         std::unique_lock lock(m_grid_lock);

    //         auto grid = m_grid.find(GridLoc(entity));

    //         if(grid != m_grid.end())
    //         {
    //             grid->second.erase(entity);
    //         }
    //     }

    //     GridCoordType GridLoc(SPCR<INamedEntity> entity)
    //     {
    //         return GridLoc(entity->GetLocation().m_loc);
    //     }
    //     GridCoordType GridLoc(CR<Location> loc)
    //     {
    //         return std::make_tuple<long, long, long>(
    //             loc.comp.x / GridSize,
    //             loc.comp.y / GridSize,
    //             loc.comp.z / GridSize
    //         );
    //     }

    //     std::shared_mutex m_grid_lock;
    //     std::map<GridCoordType, std::unordered_set<SP<INamedEntity>>> m_grid;
    // };
}
