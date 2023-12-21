#pragma once

#include <asio.hpp>
#include <function2/function2.hpp>
#include <optional>
#include <pgwire/protocol.hpp>
#include <pgwire/types.hpp>
#include <pgwire/writer.hpp>

namespace pgwire {

using Value = std::string;
using Values = std::vector<Value>;

using ExecHandler =
    fu2::unique_function<void(Writer &writer, Values const &arguments)>;

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

    void start(ParseHandler &&handler);
    void process_message(ParseHandler &handler, FrontendMessagePtr msg);

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
