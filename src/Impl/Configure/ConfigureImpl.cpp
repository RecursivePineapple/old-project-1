
#pragma target server

#include "Impl/Configure/ConfigureImpl.hpp"

namespace configure
{
    void ConfigureTestEntity(Hypodermic::ContainerBuilder &container);
    void ConfigureIPlayer(Hypodermic::ContainerBuilder &container);
    void ConfigureIDispatcher(Hypodermic::ContainerBuilder &container);
    void ConfigureIServer(Hypodermic::ContainerBuilder &container);
    void ConfigureDatabase(Hypodermic::ContainerBuilder &container);
    void ConfigureIWorldTracker(Hypodermic::ContainerBuilder &container);
    void ConfigureIUpdateTransmitter(Hypodermic::ContainerBuilder &container);
    void ConfigureIRequestDispatcher(Hypodermic::ContainerBuilder &container);
    void ConfigureIDebugSubsystem(Hypodermic::ContainerBuilder &container);
    void ConfigureIPlayerSubsystem(Hypodermic::ContainerBuilder &container);
    void ConfigureImpl(Hypodermic::ContainerBuilder &container)
    {
        ConfigureTestEntity(container);
        ConfigureIPlayer(container);
        ConfigureIDispatcher(container);
        ConfigureIServer(container);
        ConfigureDatabase(container);
        ConfigureIWorldTracker(container);
        ConfigureIUpdateTransmitter(container);
        ConfigureIRequestDispatcher(container);
        ConfigureIDebugSubsystem(container);
        ConfigureIPlayerSubsystem(container);
    }
}
