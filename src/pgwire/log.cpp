#include <cstdarg>
#include <stdexcept>

#include <pgwire/log.hpp>
#include <pgwire/utils.hpp>

#include <asio/writable_pipe.hpp>

namespace pgwire::log {

static std::unique_ptr<Writer> writer = nullptr;

enum class Level {
    Info,
    Warning,
    Error,
};

std::string now_rfc3339() {
    // https://stackoverflow.com/a/48772690
    time_t now;
    time(&now);
    struct tm *p = localtime(&now);
    char buf[100];
    size_t len = strftime(buf, sizeof buf - 1, "%FT%T%z", p);
    // move last 2 digits
    if (len > 1) {
        char minute[] = {buf[len - 2], buf[len - 1], '\0'};
        sprintf(buf + len - 2, ":%s", minute);
    }

    // add one to account the colon (:)
    return std::string(buf, len + 1);
}

Promise log(Level level, char const *format, va_list args) {
    if (!writer) {
        return resolve();
    }

    std::string message = string_format_args(format, args);
    std::string now = now_rfc3339();
    char const *level_str = "INFO";
    switch (level) {
    case Level::Warning:
        level_str = "WARNING";
        break;
    case Level::Error:
        level_str = "ERROR";
        break;
    default:
        break;
    }

    return writer->write(string_format("[%s] %s - %s\n", level_str, now.c_str(),
                                       message.c_str()));
}

Promise error(char const *format, ...) {
    va_list args;
    va_start(args, format);
    auto promise = log(Level::Error, format, args);
    va_end(args);

    return promise;
}

Promise warning(char const *format, ...) {
    va_list args;
    va_start(args, format);
    auto promise = log(Level::Warning, format, args);
    va_end(args);

    return promise;
}

Promise info(char const *format, ...) {
    va_list args;
    va_start(args, format);
    auto promise = log(Level::Info, format, args);
    va_end(args);

    return promise;
}

void initialize(asio::io_context &context, const char *file) {
    if (file == nullptr) {
        writer = std::make_unique<StreamWriter>(context, stderr);
    } else {
        writer = std::make_unique<StreamWriter>(context, file, "a");
    }
}

Writer &get_writer() {
    if (!writer) {
        throw std::runtime_error("log not yet initialized");
    }

    return *writer;
}

} // namespace pgwire::log
