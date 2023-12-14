#pragma once

#include "pgwire/pg_wire.hpp"
#include <asio.hpp>

#include <memory>
#include <optional>

namespace pgwire {

class Session : public std::enable_shared_from_this<Session> {
  public:
    Session(asio::ip::tcp::socket &&socket);
    ~Session();

    void start();

  private:
    FrontendMessagePtr read();
    FrontendMessagePtr read_startup();
    void write(Encoder const &encoder);

  private:
    bool _running;
    bool _startup_done;
    asio::ip::tcp::socket _socket;
};

class Server {
  public:
    Server(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint);
    void start();

  private:
    void accept();

  private:
    asio::io_context &_io_context;
    asio::ip::tcp::acceptor _acceptor;
};
} // namespace pgwire
