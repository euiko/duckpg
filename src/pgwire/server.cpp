#include <atomic>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <unordered_map>

#include <pgwire/io.hpp>
#include <pgwire/log.hpp>
#include <pgwire/promise.hpp>
#include <pgwire/protocol.hpp>
#include <pgwire/server.hpp>
#include <pgwire/types.hpp>
#include <pgwire/utils.hpp>

#include <asio.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <endian/network.hpp>

namespace pgwire {

using PromisePtr = std::shared_ptr<Promise>;

static std::atomic<std::size_t> sess_id_counter = 0;

class ServerImpl {
  public:
    ServerImpl(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint,
               Handler &&handler);
    void do_accept();

  private:
    friend class Server;

    asio::io_context &_io_context;
    asio::ip::tcp::acceptor _acceptor;
    Handler _handler;
    std::unordered_map<SessionID, SessionPtr> _sessions;
};

Server::Server(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint,
               Handler &&handler)
    : _impl(std::make_unique<ServerImpl>(io_context, endpoint,
                                         std::move(handler))) {}

Server::~Server() = default;

void Server::start() {
    _impl->do_accept();
    _impl->_io_context.run();
}

ServerImpl::ServerImpl(asio::io_context &io_context,
                       asio::ip::tcp::endpoint endpoint, Handler &&handler)
    : _io_context{io_context}, _acceptor{io_context, endpoint},
      _handler(std::move(handler)) {};

void ServerImpl::do_accept() {
    _acceptor.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                SessionID id = ++sess_id_counter;
                log::info("[session #%d] started", id);
                auto session = std::make_shared<Session>(id, std::move(socket));
                session->set_handler(_handler(*session));
                auto promise = session->start().finally([this, session] {
                    log::info("[session #%d] done", session->id());
                    _sessions.erase(session->id());
                });

                _sessions.emplace(id, session);
            }

            do_accept();
        });
}

} // namespace pgwire
