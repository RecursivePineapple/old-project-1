
#include <thread>

#include <uWebSockets/App.h>

#include <spdlog/spdlog.h>

#include "Common/Hypodermic.hpp"

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

    Hypodermic::Logger::configureLogLevel(Hypodermic::LogLevels::Debug);
    Hypodermic::Logger::configureSink(std::make_shared<LoggerSink>());

    Hypodermic::ContainerBuilder builder;

    configure::ConfigureImpl(builder);

    auto container = builder.build();

    std::vector<std::thread> threads;

    for(size_t i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        threads.emplace_back([&]()
        {
            uWS::App()
                .ws<server::ConnectionContext>("/*", container->resolve<server::IServer>()->Behavior())
                .listen(9001, [](auto *listen_socket) {
                    if (listen_socket) {
                        std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << 9001 << std::endl;
                    } else {
                        std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port 9001" << std::endl;
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
