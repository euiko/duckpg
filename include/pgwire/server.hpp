#pragma once

#include <asio.hpp>
#include <pgwire/protocol.hpp>
#include <pgwire/types.hpp>

namespace pgwire {

class Session : public std::enable_shared_from_this<Session> {
  public:
    Session(asio::ip::tcp::socket &&socket);
    ~Session();

    void start();

  private:
    FrontendMessagePtr read();
    FrontendMessagePtr read_startup();
    void write(Bytes &&b);

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
