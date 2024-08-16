#pragma once

#include <pgwire/session.hpp>

namespace pgwire {

using Handler = std::function<ParseHandler(Session &session)>;

class Server {
  public:
    Server(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint,
           Handler &&handler);
    ~Server();
    void start();

  private:
    void do_accept();

    asio::io_context &_io_context;
    asio::ip::tcp::acceptor _acceptor;
    Handler _handler;
    std::unordered_map<SessionID, SessionPtr> _sessions;
};
} // namespace pgwire
