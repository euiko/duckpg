#include <pgwire/exception.hpp>
#include <pgwire/utils.hpp>

namespace pgwire {

SqlException::SqlException(std::string message, SqlState state,
                           ErrorSeverity severity)
    : _error_message(std::move(message)), _error_severity(severity),
      _error_sqlstate(state) {
    message = string_format(
        "SqlException occured with severity = %s, sqlstate = %s, message = %s",
        get_error_severity(_error_severity), get_sqlstate_code(_error_sqlstate),
        _error_message.c_str());
}

const char *SqlException::what() const noexcept { return _message.c_str(); }

} // namespace pgwire
