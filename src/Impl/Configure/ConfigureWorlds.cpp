
#pragma target server

#include "ConfigureWorlds.hpp"

namespace configure
{
    void ConfigureHubWorld(Hypodermic::ContainerBuilder &container);
    void ConfigureMainWorld(Hypodermic::ContainerBuilder &container);

    void ConfigureWorlds(Hypodermic::ContainerBuilder &container)
    {
        ConfigureHubWorld(container);
        // ConfigureMainWorld(container);
    }
}
