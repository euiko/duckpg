#pragma once

#include <pgwire/session.hpp>

namespace pgwire {

using Handler = std::function<ParseHandler(Session &session)>;
class ServerImpl;
class Server {
  public:
    Server(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint,
           Handler &&handler);
    ~Server();
    void start();

  private:
    std::unique_ptr<ServerImpl> _impl;
};

} // namespace pgwire
