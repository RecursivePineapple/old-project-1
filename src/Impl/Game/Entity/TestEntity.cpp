
#pragma target server

#include "Interface/Game/INamedEntity.hpp"
#include "Interface/Game/IPlayer.hpp"

#include "BasicNamedEntity.hpp"

namespace gamestate
{
    #define TEST_STATE_FIELDS(X) \
        X(int, jsontypes::integer, data)

    DECLARE_JSON_STRUCT(TestState, TEST_STATE_FIELDS)

    struct TestEntity : public IBasicNamedEntity<TestState>
    {
        virtual std::string EntityType() override { return "TestEntity"; }

        virtual void OnMessage(ConnectionContext *sender, CR<std::string> action, std::optional<jsontypes::span_t> data) override
        {
            (void)sender;
            (void)data;

            if(action == "ping")
            {
                auto state = this->m_state;
                state.data++;
                this->SetState(state);
            }
        }
    };
}

#include <Hypodermic/ContainerBuilder.h>

namespace configure
{
    void ConfigureTestEntity(Hypodermic::ContainerBuilder &container);
    void ConfigureTestEntity(Hypodermic::ContainerBuilder &container)
    {
        container
            .registerType<gamestate::TestEntity>()
            .named<gamestate::INamedEntity>("TestEntity");
    }
}
