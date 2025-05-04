
#pragma target server

#include <string>
#include <memory>
#include <spdlog/spdlog.h>

#include "Common/Types.hpp"

#include "Interface/Server/IServer.hpp"
#include "Interface/Server/IDispatcher.hpp"
#include "Interface/Subsystem/IPlayerSubsystem.hpp"

namespace server
{
    struct Connection : public IConnection
    {
        Connection(IServer::SocketType *ws) : m_socket(ws) { }

        virtual void SendUnsafe(CR<std::string> msg) override
        {
            m_socket->send(msg, uWS::OpCode::TEXT);
            spdlog::debug("[{0}] sending: {1}", m_socket->getRemoteAddressAsText(), msg);
        }

    private:
        IServer::SocketType *m_socket;
    };

    struct Server : public IServer
    {
        using IServer::BehaviorType;
        using IServer::SocketType;
        Server(
            SP<IDispatcher> dispatcher,
            SP<IPlayerSubsystem> playersub)
          : m_dispatcher(dispatcher),
            m_player_subsystem(playersub)
        { }

        virtual BehaviorType Behavior() override
        {
            BehaviorType b;
            b.compression = uWS::SHARED_COMPRESSOR;
            b.maxPayloadLength = 16 * 1024;
            b.idleTimeout = 16;
            b.maxBackpressure = 1 * 1024 * 1024;
            b.resetIdleTimeoutOnSend = true;
            b.upgrade = nullptr;

            b.open = [this](SocketType *ws) noexcept
            {
                this->OnOpen(ws);
            };

            b.message = [this](SocketType *ws, std::string_view message, uWS::OpCode code) noexcept
            {
                this->OnMessage(ws, message, code);
            };

            b.close = [this](SocketType *ws, int code, std::string_view message) noexcept
            {
                this->OnClose(ws, code, message);
            };
            
            return b;
        }

    private:
        void OnOpen(SocketType *ws) noexcept
        {
            auto data = ws->getUserData();

            data->m_remote_address = ws->getRemoteAddressAsText();
            spdlog::debug("received connection from: {0}", data->m_remote_address);
            
            data->m_player = m_player_subsystem->CreateUnathenticated();
            data->m_player->SetConnection(std::make_shared<Connection>(ws));
        }

        void OnMessage(SocketType *ws, CR<std::string_view> message, uWS::OpCode) noexcept
        {
            auto data = ws->getUserData();

            spdlog::debug("[{0}] received message: {1}", data->m_remote_address, message);
            m_dispatcher->Dispatch(data, message);
        }

        void OnClose(SocketType *ws, int, CR<std::string_view>) noexcept
        {
            auto data = ws->getUserData();

            spdlog::debug("closing connection to: {0}", data->m_remote_address);
            m_player_subsystem->Disconnect(data->m_player);
        }

        SP<IDispatcher> m_dispatcher;
        SP<IPlayerSubsystem> m_player_subsystem;
    };

}

#include <Hypodermic/ContainerBuilder.h>

#pragma configurable ConfigureIServer

namespace configure
{
    void ConfigureIServer(Hypodermic::ContainerBuilder &container);
    void ConfigureIServer(Hypodermic::ContainerBuilder &container)
    {
        container.registerType<server::Server>().as<server::IServer>().singleInstance();
    }
}
