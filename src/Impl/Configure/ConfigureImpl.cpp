
#pragma target server

#include "ConfigureImpl.hpp"

#include "Impl/Configure/ConfigureWorlds.hpp"
#include "Impl/Configure/ConfigureEntities.hpp"

namespace configure
{
    void ConfigureIServer(Hypodermic::ContainerBuilder &container);
    void ConfigureIDispatcher(Hypodermic::ContainerBuilder &container);
    void ConfigureDatabase(Hypodermic::ContainerBuilder &container);

    void ConfigureIPlayerSubsystem(Hypodermic::ContainerBuilder &container);
    void ConfigureIPlayer(Hypodermic::ContainerBuilder &container);
    
    void ConfigureIWorldTracker(Hypodermic::ContainerBuilder &container);

    void ConfigureIDebugSubsystem(Hypodermic::ContainerBuilder &container);

    // void ConfigureIPlayerInfoLookup(Hypodermic::ContainerBuilder &container);
    // void ConfigureIEntityFactory(Hypodermic::ContainerBuilder &container);
    // void ConfigureIWorldTracker(Hypodermic::ContainerBuilder &container);
    // void ConfigureIWorldSubsystem(Hypodermic::ContainerBuilder &container);

    void ConfigureImpl(Hypodermic::ContainerBuilder &container)
    {
        ConfigureIServer(container);
        ConfigureIDispatcher(container);

        ConfigureDatabase(container);

        ConfigureIPlayerSubsystem(container);
        ConfigureIPlayer(container);

        ConfigureIWorldTracker(container);

        ConfigureWorlds(container);
        ConfigureEntities(container);

        ConfigureIDebugSubsystem(container);

        // ConfigureIPlayerInfoLookup(container);
        // ConfigureIEntityFactory(container);
        // ConfigureIWorldTracker(container);
        // ConfigureIWorldSubsystem(container);
    }
}
