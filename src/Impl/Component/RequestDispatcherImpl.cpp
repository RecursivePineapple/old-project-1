
#pragma target server

#include "Utils/env.hpp"
#include "Utils/http.hpp"

#include "Interface/Component/IRequestDispatcher.hpp"

namespace gamestate
{
    struct RequestDispatcher : public IRequestDispatcher
    {
        virtual std::future<std::optional<std::string>> GetLocalAddress()
        {
            std::lock_guard l(m_addr_mutex);

            if(m_local_address)
            {
                std::promise<std::optional<std::string>> p;
                p.set_value(m_local_address.value());
                return p.get_future();
            }

            return std::async([this]()
            {
                auto resp = utils::DoGet(utils::getenv("MGMT_SERVICE", "https://localhost:8082/") + "/network/ip");

                std::string addr;
                if(resp && server::ParseValue<jsontypes::string, std::string>(resp.value(), "address", addr))
                {
                    std::lock_guard l(this->m_addr_mutex);

                    this->m_local_address = addr;

                    return this->m_local_address;
                }
                else
                {
                    return std::optional<std::string>();
                }
            });
        }
    
    private:
        std::mutex m_addr_mutex;
        std::optional<std::string> m_local_address;
    };
}

#include <Hypodermic/ContainerBuilder.h>

#pragma configurable ConfigureIRequestDispatcher

namespace configure
{
    void ConfigureIRequestDispatcher(Hypodermic::ContainerBuilder &container)
    {
        container.registerType<gamestate::RequestDispatcher>().as<gamestate::IRequestDispatcher>().singleInstance();
    }
}
