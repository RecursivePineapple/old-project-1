
#pragma target server

#include <Hypodermic/ContainerBuilder.h>

#include "Common/Types.hpp"

#include "Utils/dbwrapper/libpq.hpp"

namespace configure
{
    namespace pq = dbwrapper::libpq;

    static pq::connection_info GetConnectionInfo()
    {
        return pq::connection_info(
            "postgres",
            "rootpassword"
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
