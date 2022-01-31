
#pragma target server

#include "Interface/Game/INamedEntity.hpp"
#include "Interface/Game/IPlayer.hpp"

#include "BasicNamedEntity.hpp"

namespace gamestate
{
    #define TEST_RESP_FIELDS(X) \
        X(std::string, jsontypes::string, foo)

    DECLARE_JSON_STRUCT(TestResp, TEST_RESP_FIELDS)

    struct TestEntity : public IBasicNamedEntity<>
    {
        virtual std::string EntityType() override { return "TestEntity"; }

        virtual void OnMessage(ConnectionContext *sender, CR<gamestate::EntityMessage> msg) override
        {
            if(msg.action == "ping")
            {
                TestResp resp;
                resp.foo = "pong";

                sender->m_player->GetConnection()->Send(server::StringMessage::FromValue(resp));
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
