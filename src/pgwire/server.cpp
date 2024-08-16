
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <sstream>
#include <unordered_map>

#include <pgwire/io.hpp>
#include <pgwire/log.hpp>
#include <pgwire/protocol.hpp>
#include <pgwire/server.hpp>
#include <pgwire/types.hpp>
#include <pgwire/utils.hpp>

#include <asio.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <endian/network.hpp>

namespace pgwire {

static std::atomic<std::size_t> sess_id_counter = 0;

Server::Server(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint,
               Handler &&handler)
    : _io_context{io_context}, _acceptor{io_context, endpoint},
      _handler(std::move(handler)) {};

Server::~Server() = default;

void Server::start() {
    this->do_accept();
    _io_context.run();
}

void Server::do_accept() {
    _acceptor.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                SessionID id = ++sess_id_counter;
                log::info("session %d started", id);
                auto session = std::make_shared<Session>(std::move(socket));
                session->set_handler(_handler(*session));
                session->start().then([id, this] {
                    log::info("session %d done", id);
                    _sessions.erase(id);
                });

                _sessions.emplace(id, session);
            }

            do_accept();
        });
}

} // namespace pgwire
