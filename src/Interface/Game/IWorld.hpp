
#pragma once

#include <set>
#include <unordered_set>

#include "Utils/uuid.hpp"

#include "Common/Types.hpp"
#include "Common/Location.hpp"
#include "Common/Message.hpp"

#include "Interface/Game/INamedEntity.hpp"
#include "Interface/Game/ISession.hpp"

namespace gamestate
{
    struct EntityLockResult
    {
        std::vector<ISession*> m_lock_conflicts;
        SP<std::map<uuid, SP<INamedEntity>>> m_successfully_locked;
        SP<std::unordered_set<uuid>> m_ids;
        
        EntityLockResult(
            std::vector<ISession*> &&conflicts,
            std::map<uuid, SP<INamedEntity>> &&successes,
            std::unordered_set<uuid> &&ids
        ) : m_lock_conflicts(std::move(conflicts)),
            m_successfully_locked(std::make_shared<std::map<uuid, SP<INamedEntity>>>(std::move(successes))),
            m_ids(std::make_shared<std::unordered_set<uuid>>(std::move(ids)))
            { }
    };

    struct IWorld
    {
        virtual ~IWorld() { }

        virtual std::string WorldType() = 0;

        virtual EntityLockResult TryLockEntitiesWithin(CR<Location> loc, long size, ISession* locker) = 0;

        virtual SP<INamedEntity> FindEntity(CR<uuid> id) = 0;

        virtual SP<INamedEntity> Spawn(
            CR<std::string> type,
            CR<INamedEntity::LocationType> loc,
            ISession* locker = nullptr) = 0;
        virtual void Remove(SPCR<INamedEntity> entity) = 0;
        virtual void Destroy(SPCR<INamedEntity> entity) = 0;

        virtual void Dispatch(CR<server::Message> msg) = 0;

        virtual void OnEntityMoved(CR<uuid> id, CR<Location> old_loc) = 0;

        virtual ISession* FindSessionAt(CR<Location> loc) = 0;

        virtual void Load() = 0;
        virtual void Save() = 0;

        virtual void Generate() = 0;
    public:

        virtual CR<uuid> GetId() { return m_id; }
        virtual void SetId(CR<uuid> id) { m_id = id; }

        virtual bool IsTransient() { return m_is_transient; }
        virtual void SetTransient(bool transient) { m_is_transient = transient; }
    protected:
        uuid m_id;
        bool m_is_transient;
    };
}
