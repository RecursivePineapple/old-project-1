
#pragma once

#include <map>
#include <memory>
#include <functional>

namespace gamestate
{
    template<typename... Args>
    struct Event
    {
        template<auto handler, typename TOwner>
        int Bind(TOwner *owner)
        {
            int id = ++m_next_id;
            m_handlers[id] = [=](const Args&... args)
            {
                (owner->*handler)(args...);
                return true;
            };
            return id;
        }

        template<auto handler, typename TOwner>
        int Bind(TOwner const& owner)
        {
            int id = ++m_next_id;
            m_handlers[id] = [owner = std::move(owner)](const Args&... args)
            {
                (owner.*handler)(args...);
                return true;
            };
            return id;
        }

        template<auto handler, typename TOwner>
        int Bind(std::shared_ptr<TOwner> const& owner)
        {
            int id = ++m_next_id;
            m_handlers[id] = [owner = std::move(owner)](const Args&... args)
            {
                if(owner)
                {
                    (owner.*handler)(args...);
                    return true;
                }
                else
                {
                    return false;
                }
            };
            return id;
        }

        template<auto handler, typename TOwner>
        int Bind(std::weak_ptr<TOwner> const& owner)
        {
            int id = ++m_next_id;
            m_handlers[id] = [owner = std::move(owner)](const Args&... args)
            {
                if(owner)
                {
                    (owner.*handler)(args...);
                    return true;
                }
                else
                {
                    return false;
                }
            };
            return id;
        }

        void Unbind(int id)
        {
            m_handlers.erase(id);
        }

        void Dispatch(const Args&... args)
        {
            for(auto iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
            {
                if(!iter->second(args...))
                {
                    iter = m_handlers.erase(iter);
                }
            }
        }

        int m_next_id;
        std::map<int, std::function<bool(const Args&... args)>> m_handlers;
    };
}
