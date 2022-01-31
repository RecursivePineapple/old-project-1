
#pragma once

#include <set>
#include <unordered_set>

#include "Utils/uuid.hpp"

#include "Common/Types.hpp"
#include "Common/Transform.hpp"
#include "Common/Message.hpp"
#include "Common/EntityMessage.hpp"

#include "Interface/Game/INamedEntity.hpp"

namespace gamestate
{

    struct IWorld
    {
        virtual ~IWorld() { }

        virtual std::string WorldType() = 0;

        virtual SP<INamedEntity> FindEntity(CR<uuid> id) = 0;

        virtual SP<INamedEntity> Spawn(CR<std::string> type, CR<INamedEntity::TransformType> loc) = 0;
        virtual void Remove(SPCR<INamedEntity> entity) = 0;
        virtual void Destroy(SPCR<INamedEntity> entity) = 0;

        virtual void AddPlayer(SPCR<IPlayer> player) = 0;
        virtual void RemovePlayer(SPCR<IPlayer> player) = 0;
        
        virtual void Dispatch(ConnectionContext *sender, CR<gamestate::EntityMessage> msg) = 0;

        virtual void Load() = 0;
        virtual void Save() = 0;

        virtual void Generate() = 0;

    public:

        virtual CR<uuid> GetId() { return m_world_id; }
        virtual void SetId(CR<uuid> id) { m_world_id = id; }

        virtual bool IsTransient() { return m_is_transient; }
        virtual void SetTransient(bool transient) { m_is_transient = transient; }
    protected:
        uuid m_world_id;
        bool m_is_transient;
    };
}
