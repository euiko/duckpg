#pragma once

#include <asio.hpp>
#include <pgwire/protocol.hpp>
#include <pgwire/types.hpp>
#include <pgwire/writer.hpp>

namespace pgwire {

using Value = std::string;
using Values = std::vector<Value>;

using ExecHandler =
    std::function<void(Writer &writer, Values const &arguments)>;

struct PreparedStatement {
    Fields fields;
    ExecHandler handler;
};

using ParseHandler = std::function<PreparedStatement(std::string const &)>;
class Session {
  public:
    Session(asio::ip::tcp::socket &&socket);
    ~Session();

    void start(ParseHandler &&handler);

  private:
    FrontendMessagePtr read();
    FrontendMessagePtr read_startup();
    void write(Bytes &&b);

  private:
    bool _running;
    bool _startup_done;
    asio::ip::tcp::socket _socket;
};

using Handler = std::function<ParseHandler(Session &session)>;

class Server {
  public:
    Server(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint,
           Handler &&handler);
    void start();

  private:
    void accept();

  private:
    asio::io_context &_io_context;
    asio::ip::tcp::acceptor _acceptor;
    Handler _handler;
};
} // namespace pgwire
