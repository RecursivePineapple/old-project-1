
#pragma target server

#include <shared_mutex>
#include <vector>
#include <map>

#include "Utils/uuid.hpp"

#include "Interface/Game/ISession.hpp"
#include "Interface/Game/IWorld.hpp"
#include "Interface/Game/IPlayer.hpp"

#include "HubSession.hpp"

namespace gamestate
{
    struct IPlayer;
    struct INamedEntity;

    struct HubSession : public ISession, public IHubSession
    {
        virtual void AddPlayer(SPCR<IPlayer> player) override
        {
            std::unique_lock lock(m_player_mutex);

            m_players[player->GetEntity()->GetId()] = player;
            player->SetSession(this);
            
            std::vector<server::StringMessage> msgs;

            using server::StringMessage;

            msgs.push_back(StringMessage::Object({
                {"type", "load-world"},
                {"world-type", m_world->WorldType()},
                {"world-id", m_world->GetId().to_string()}
            }));

            for(auto const& e : m_locked_entities)
            {
                auto const& l = e.second->GetLocation().m_loc;

                msgs.push_back(StringMessage::Object({
                    {"type", "create-entity"},
                    {"eid", e.first.to_string()},
                    {"etype", e.second->EntityType()},
                    {"x", std::to_string(l.comp.x)},
                    {"y", std::to_string(l.comp.y)},
                    {"z", std::to_string(l.comp.z)},
                    {"state", e.second->SaveState()}
                }));
            }

            msgs.push_back(StringMessage::Object({
                {"type", "possess"},
                {"id", player->GetEntity()->GetId().to_string()}
            }));

            player->GetConnection()->SendBulk(msgs);
        }

        virtual void RemovePlayer(SPCR<IPlayer> player) override
        {
            std::unique_lock lock(m_player_mutex);

            m_players.erase(player->GetEntity()->GetId());
        }

        virtual void OnPlayerLocationChanged(SPCR<IPlayer> player) override
        {
            (void)player;
        }

        virtual void SetLockedEntities(CR<std::unordered_set<SP<INamedEntity>>> entities) override
        {
            std::lock_guard lock(m_entity_mutex);

            m_locked_entities.clear();
            m_locked_ids.clear();

            for(auto const& e : entities)
            {
                m_locked_entities[e->GetId()] = e;
                m_locked_ids.insert(e->GetId());
            }
        }

        virtual void Broadcast(CR<server::Message> msg) override
        {
            std::shared_lock lock(m_player_mutex);

            for(auto const& p : m_players)
            {
                p.second->GetConnection()->Send(msg);
            }
        }

        virtual IWorld* GetWorld() override { return m_world; }
        virtual void SetWorld(IWorld *world) override { m_world = world; }

        IWorld *m_world;

        std::shared_mutex m_player_mutex;
        std::map<uuid, SP<IPlayer>> m_players;
        
        std::mutex m_entity_mutex;
        std::map<uuid, SP<INamedEntity>> m_locked_entities;
        std::unordered_set<uuid> m_locked_ids;
    };
}

#include <Hypodermic/ContainerBuilder.h>

namespace configure
{
    void ConfigureHubSession(Hypodermic::ContainerBuilder &container);
    void ConfigureHubSession(Hypodermic::ContainerBuilder &container)
    {
        container
            .registerType<gamestate::HubSession>()
            .as<gamestate::ISession>()
            .named("HubSession");
    }
}
