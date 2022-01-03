
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

        virtual void Send(const Message *msg) override
        {
            m_socket->send(msg->Text(), uWS::OpCode::TEXT);
            spdlog::debug("[{0}] sending: {1}", m_socket->getRemoteAddressAsText(), msg->Text());
        }

        virtual void SendBulk(const Message *data, size_t n) override
        {
            std::stringstream ss;

            ss << "[";

            for(size_t i = 0; i < n; ++i)
            {
                switch(i)
                {
                    default:
                        ss << ",";
                        goto zero;
                        break;
                    case 0:
                        zero:
                        ss << data[i].Text();
                        break;
                }
            }

            ss << "]";

            m_socket->send(ss.str(), uWS::OpCode::TEXT);
            spdlog::debug("[{0}] sending: {1}", m_socket->getRemoteAddressAsText(), ss.str());
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
            spdlog::debug("received connection from: {0}", ws->getRemoteAddressAsText());
            ws->getUserData()->m_player = m_player_subsystem->CreateUnathenticated();
            ws->getUserData()->m_player->SetConnection(std::make_shared<Connection>(ws));
        }

        void OnMessage(SocketType *ws, CR<std::string_view> message, uWS::OpCode) noexcept
        {
            spdlog::debug("[{0}] received message: {1}", ws->getRemoteAddressAsText(), message);
            m_dispatcher->Dispatch(ws->getUserData(), message);
        }

        void OnClose(SocketType *ws, int, CR<std::string_view>) noexcept
        {
            spdlog::debug("closing connection to: {0}", ws->getRemoteAddressAsText());
            m_player_subsystem->Disconnect(ws->getUserData()->m_player);
        }

        SP<IDispatcher> m_dispatcher;
        SP<IPlayerSubsystem> m_player_subsystem;
    };

}

#include <Hypodermic/ContainerBuilder.h>

namespace configure
{
    void ConfigureIServer(Hypodermic::ContainerBuilder &container);
    void ConfigureIServer(Hypodermic::ContainerBuilder &container)
    {
        container.registerType<server::Server>().as<server::IServer>().singleInstance();
    }
}
