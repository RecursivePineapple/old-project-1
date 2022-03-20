
#include <thread>
#include <spdlog/spdlog.h>
#include <uWebSockets/App.h>
#include <Hypodermic/ContainerBuilder.h>

#include "Utils/env.hpp"
#include "Utils/jsonstruct.hpp"

#include "Interface/Server/IServer.hpp"

#include "Impl/Configure/ConfigureImpl.hpp"

struct LoggerSink : Hypodermic::ILoggerSink
{
    virtual void append(Hypodermic::LogLevels::LogLevel level, const std::string& message) override
    {
        switch(level)
        {
            case Hypodermic::LogLevels::Debug: {
                spdlog::debug(message);
                break;
            }
            case Hypodermic::LogLevels::Info: {
                spdlog::info(message);
                break;
            }
            case Hypodermic::LogLevels::Warn: {
                spdlog::warn(message);
                break;
            }
            case Hypodermic::LogLevels::Error: {
                spdlog::error(message);
                break;
            }
            default: {
                break;
            }
        }
    }
};

int main(int argc, const char **argv)
{
    (void)argc;
    (void)argv;

    spdlog::set_level(spdlog::level::trace);

    Hypodermic::Logger::configureLogLevel(Hypodermic::LogLevels::Debug);
    Hypodermic::Logger::configureSink(std::make_shared<LoggerSink>());

    Hypodermic::ContainerBuilder builder;

    configure::ConfigureImpl(builder);

    auto container = builder.build();

    std::vector<std::thread> threads;

    int port = 0;

    if(!jsontypes::ParseJson<jsontypes::integer>(utils::getenv("WS_PORT", "9001"), port))
    {
        spdlog::error("Illegal port {0}. Exiting.", utils::getenv("WS_PORT", "9001"));
        return 1;
    }

    for(size_t i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        threads.emplace_back([&container, i, port]()
        {
            uWS::App()
                .ws<ConnectionContext>("/*", container->resolve<server::IServer>()->Behavior())
                .listen(port, [i](auto *listen_socket) {
                    if(listen_socket != nullptr)
                    {
                        spdlog::info("thread {0} listening on port {1}", i, 9001);
                    }
                    else
                    {
                        spdlog::error("thread {0} could not listen on port {1}", i, 9001);
                    }
                })
                .run();
        });
    }

    for(auto &t : threads)
    {
        t.join();
    }

    return 0;
}
