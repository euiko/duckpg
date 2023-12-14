#pragma once

#include <asio.hpp>

#include <memory>
#include <optional>

namespace duckpg {

class Session : public std::enable_shared_from_this<Session> {
  public:
    Session(asio::ip::tcp::socket &&socket);
    ~Session();

    void start();

  private:
  private:
    asio::ip::tcp::socket _socket;
};

class Server {
  public:
    Server(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint);
    void start();

  private:
    void async_accept();

  private:
    asio::io_context &_io_context;
    asio::ip::tcp::acceptor _acceptor;
    std::optional<asio::ip::tcp::socket> _socket;
};
} // namespace duckpg
