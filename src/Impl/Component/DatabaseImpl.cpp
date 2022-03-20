
#pragma target server

#include <Hypodermic/ContainerBuilder.h>

#include "Common/Types.hpp"

#include "Utils/dbwrapper/libpq.hpp"
#include "Utils/env.hpp"

namespace configure
{
    namespace pq = dbwrapper::libpq;

    static pq::connection_info GetConnectionInfo()
    {
        return pq::connection_info(
            utils::getenv("PG_USER", "postgres"),
            utils::getenv("PG_PWD", "rootpassword"),
            utils::getenv("PG_HOST", "localhost")
        );
    }

    void ConfigureDatabase(Hypodermic::ContainerBuilder &container);
    void ConfigureDatabase(Hypodermic::ContainerBuilder &container)
    {
        container
            .registerType<pq::database>()
            .onActivated([](auto, auto inst)
            {
                inst->open(GetConnectionInfo());
            });
    }
}
