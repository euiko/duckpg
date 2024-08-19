#pragma once

#include <optional>

#include <pgwire/io.hpp>
#include <pgwire/protocol.hpp>
#include <pgwire/types.hpp>
#include <pgwire/writer.hpp>

#include <asio.hpp>
#include <function2/function2.hpp>

namespace pgwire {

// forward declaration
class Server;
class ServerImpl;
class Session;
struct PreparedStatement;

using Value = std::string;
using Values = std::vector<Value>;
using ExecHandler =
    fu2::unique_function<void(Writer &writer, Values const &arguments)>;
using ParseHandler = std::function<PreparedStatement(std::string const &)>;
using SessionID = std::size_t;
using SessionPtr = std::shared_ptr<Session>;

struct PreparedStatement {
    Fields fields;
    ExecHandler handler;
};

class Session {
  public:
    Session(SessionID id, asio::ip::tcp::socket &&socket);
    ~Session();

    Promise start();
    Promise process_message(FrontendMessagePtr msg);
    SessionID id() const;

  private:
    void set_handler(ParseHandler &&handler);
    void do_read(Defer defer);
    Promise read();
    Promise read_startup();
    Promise write(Bytes &&b);

  private:
    friend class Server;
    friend class ServerImpl;

    SessionID _id;
    bool _startup_done;
    asio::ip::tcp::socket _socket;
    std::optional<ParseHandler> _handler;
};

} // namespace pgwire
