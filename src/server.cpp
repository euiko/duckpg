#include <duckpg/server.h>

#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>

using namespace boost::asio;

namespace duckpg {
Session::Session(ip::tcp::socket &&socket)
    : _socket{std::move(socket)} {

      };
Session::~Session() = default;

void Session::start() {}

Server::Server(io_context &io_context, ip::tcp::endpoint endpoint)
    : _io_context{io_context}, _acceptor{io_context, endpoint} {};

void Server::start() { async_accept(); }

void Server::async_accept() {
    _socket.emplace(_io_context);
    auto handler = [&](boost::system::error_code error) {
        // move socket to be managed by the session and clean the current socket
        std::make_shared<Session>(std::move(*_socket))->start();
        async_accept();
    };
    _acceptor.async_accept(*_socket, handler);
}

} // namespace duckpg