
#pragma once

#include "Common/Message.hpp"

#include "Interface/Game/INamedEntity.hpp"
#include "Interface/Game/IWorld.hpp"
#include "Interface/Game/MessageIO/Entity.hpp"

namespace gamestate
{
    #define X_NOOP_FIELDS(X)

    DECLARE_JSON_STRUCT(NoopState, X_NOOP_FIELDS)

    template<typename TState = NoopState>
    struct IBasicNamedEntity : public INamedEntity
    {
        using StateType = TState;

        virtual bool LoadState(CR<jsontypes::span_t> msg) override
        {
            if(!msg.ParseInto<TState>(m_state))
            {
                return false;
            }

            return true;
        }

        virtual std::string SaveState() override
        {
            std::stringstream ss;

            TState::emit(ss, m_state);

            return ss.str();
        }

        IMPLEMENT_PROPERTY(uuid, Id, m_id)
        IMPLEMENT_PROP_GETTER(TransformType, Location, m_transform)

        virtual void SetLocation(CR<TransformType> transform) override
        {
            auto old_loc = m_transform.m_transform.m_loc;
            m_transform = transform;
            m_move_evt.Dispatch(this);
        }

        virtual Event<INamedEntity*>& OnMove() override
        {
            return m_move_evt;
        }

        virtual Event<INamedEntity*>& OnStateChanged() override
        {
            return m_change_evt;
        }

    protected:
        TransformType m_transform;
        TState m_state;
        Event<INamedEntity*> m_move_evt;
        Event<INamedEntity*> m_change_evt;

        void SetState(CR<TState> newState)
        {
            m_state = newState;
            m_change_evt.Dispatch(this);
        }
    };
}
