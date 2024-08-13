#pragma once

#include <optional>

#include <pgwire/io.hpp>
#include <pgwire/protocol.hpp>
#include <pgwire/types.hpp>
#include <pgwire/writer.hpp>

#include <asio.hpp>
#include <function2/function2.hpp>

namespace pgwire {

using Value = std::string;
using Values = std::vector<Value>;

using ExecHandler =
    fu2::unique_function<void(Writer &writer, Values const &arguments)>;

class ServerImpl;

struct PreparedStatement {
    Fields fields;
    ExecHandler handler;
};

class SqlException : public std::exception {
  public:
    SqlException(std::string message, SqlState state,
                 ErrorSeverity severity = ErrorSeverity::Error);

    inline std::string const &get_message() const noexcept {
        return _error_message;
    }

    inline ErrorSeverity get_severity() const noexcept {
        return _error_severity;
    }

    inline SqlState get_sqlstate() const noexcept { return _error_sqlstate; }

    const char *what() const noexcept override;

  private:
    std::string _message;
    std::string _error_message;
    ErrorSeverity _error_severity = ErrorSeverity::Error;
    SqlState _error_sqlstate = SqlState::ProtocolViolation;
};

using ParseHandler = std::function<PreparedStatement(std::string const &)>;

class Session {
  public:
    Session(asio::ip::tcp::socket &&socket);
    ~Session();

    Promise start();
    Promise process_message(FrontendMessagePtr msg);

  private:
    void set_handler(ParseHandler &&handler);
    void do_read(Defer &defer);
    Promise read();
    Promise read_startup();
    Promise write(Bytes &&b);

  private:
    friend class ServerImpl;

    bool _startup_done;
    asio::ip::tcp::socket _socket;
    std::optional<ParseHandler> _handler;
};

using SessionID = std::size_t;
using SessionPtr = std::shared_ptr<Session>;
using Handler = std::function<ParseHandler(Session &session)>;

class Server {
  public:
    Server(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint,
           Handler &&handler);
    ~Server();
    void start();

  private:
    friend class ServerImpl;

    asio::io_context &_io_context;
    asio::ip::tcp::acceptor _acceptor;
    Handler _handler;
    std::unique_ptr<ServerImpl> _impl;
    std::unordered_map<SessionID, SessionPtr> _sessions;
};
} // namespace pgwire
