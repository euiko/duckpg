#pragma once

#include <exception>
#include <memory>
#include <string>

#include <pgwire/types.hpp>

namespace pgwire {

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

using SqlExceptionPtr = std::shared_ptr<SqlException>;

} // namespace pgwire
