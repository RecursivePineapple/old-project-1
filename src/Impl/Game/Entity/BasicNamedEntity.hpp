
#pragma once

#include <mutex>

#include "Interface/Game/INamedEntity.hpp"
#include "Interface/Game/IWorld.hpp"

namespace gamestate
{
    struct IBasicNamedEntity : INamedEntity
    {
        virtual bool LoadState(CR<std::string>) override { return true; }

        virtual std::string SaveState() override { return "{}"; }

        virtual std::string SaveViewState() override { return "{}"; }

        virtual bool IsLoaded() override { return true; }

        virtual CR<uuid> GetId() override { return m_id; }
        virtual void SetId(CR<uuid> id) override { m_id = id; }
        virtual ISession* GetSession() override { return m_session; }
        virtual CR<LocationType> GetLocation() override { return m_loc; }
        virtual void SetLocation(CR<LocationType> loc) override
        {
            auto old_loc = m_loc.m_loc;
            m_loc = loc;
            loc.m_world->OnEntityMoved(m_id, old_loc);

            PushState(true);
        }

        virtual ISession* TryLock(ISession *session) override
        {
            std::lock_guard lock(m_session_mutex);

            if(m_session != nullptr)
            {
                return m_session;
            }
            else
            {
                return m_session = session;
            }
        }

        virtual bool Unlock() override
        {
            std::lock_guard lock(m_session_mutex);

            m_session = nullptr;

            return true;
        }

    protected:
        std::mutex m_session_mutex;
        ISession *m_session;
        IWorld *m_world;
        uuid m_id;
        LocationType m_loc;

        void PushState(bool loc_only = false)
        {
            if(m_session != nullptr)
            {
                if(loc_only)
                {
                    m_session->Broadcast(server::StringMessage::Object({
                        {"eid", m_id.to_string()},
                        {"x", std::to_string(m_loc.m_loc.comp.x)},
                        {"y", std::to_string(m_loc.m_loc.comp.y)},
                        {"z", std::to_string(m_loc.m_loc.comp.z)}
                    }));
                }
                else
                {
                    m_session->Broadcast(server::StringMessage::Object({
                        {"eid", m_id.to_string()},
                        {"x", std::to_string(m_loc.m_loc.comp.x)},
                        {"y", std::to_string(m_loc.m_loc.comp.y)},
                        {"z", std::to_string(m_loc.m_loc.comp.z)},
                        {"state", SaveState()}
                    }));
                }
            }
        }

        template<typename TState, typename ctype = TState>
        static bool LoadJsonState(CR<std::string> msg, std::unique_ptr<ctype> &state_ptr)
        {
            TState state;

            std::vector<jsmntok_t> toks;
            if(!server::ParseJson(toks, msg))
            {
                return false;
            }

            if(!TState::parse(toks.data(), 0, msg.data(), state))
            {
                return false;
            }

            state_ptr.swap(std::unique_ptr<TState>(new TState(std::move(state))));

            return true;
        }

        template<typename TState, typename ctype = TState>
        static std::string SaveJsonState(CR<std::unique_ptr<ctype>> state_ptr)
        {
            std::stringstream ss;

            TState::emit(ss, *state_ptr);

            return ss.str();
        }
    };
}
