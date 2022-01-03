
#pragma target server

#include "ConfigureEntities.hpp"

namespace configure
{
    void ConfigureTestEntity(Hypodermic::ContainerBuilder &container);

    void ConfigureEntities(Hypodermic::ContainerBuilder &container)
    {
        ConfigureTestEntity(container);
    }
}
