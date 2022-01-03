
// #pragma target server

#include <mutex>
#include <vector>
#include <map>

#include "Utils/uuid.hpp"

#include "Interface/Game/ISession.hpp"
#include "Interface/Game/IWorld.hpp"
#include "Interface/Game/IPlayer.hpp"

namespace gamestate
{
    struct IPlayer;
    struct INamedEntity;

    struct MainSession : public ISession
    {
        const static long RenderDistance = 100 * 1000;

        virtual void AddPlayer(SPCR<IPlayer> player) override
        {
            std::lock_guard lock(m_player_mutex);

            m_players[player->GetEntity()->GetId()] = player;
        }

        virtual void RemovePlayer(SPCR<IPlayer> player) override
        {
            std::lock_guard lock(m_player_mutex);

            m_players.erase(player->GetEntity()->GetId());
        }

        virtual void OnPlayerLocationChanged(SPCR<IPlayer> player) override
        {
            auto res = m_world->TryLockEntitiesWithin(player->GetEntity()->GetLocation(), RenderDistance, this);

            
        }

        virtual IWorld* GetWorld() override { return m_world; }
        virtual void SetWorld(IWorld *world) override { m_world = world; }

        IWorld *m_world;

        std::mutex m_player_mutex;
        std::map<uuid, SP<IPlayer>> m_players;

        std::mutex m_entity_mutex;
        std::map<uuid, SP<INamedEntity>> m_locked_entities;
        std::unordered_set<uuid> m_locked_ids;
    };
}

