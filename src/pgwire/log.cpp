#include <cstdarg>
#include <stdexcept>

#include <pgwire/log.hpp>
#include <pgwire/utils.hpp>

#include <asio/writable_pipe.hpp>

namespace pgwire::log {

static std::unique_ptr<Writer> writer = nullptr;

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

    return {buf, len + 1};
}

Promise warning(char const *format, ...) {
    if (!writer) {
        return resolve();
    }

    va_list args;
    va_start(args, format);
    std::string message = string_format_args(format, args);
    va_end(args);

    std::string now = now_rfc3339();
    return writer->write(
        string_format("[WARNING] %s - %s\n", now.c_str(), message.c_str()));
}

Promise info(char const *format, ...) {
    if (!writer) {
        return resolve();
    }

    va_list args;
    va_start(args, format);
    std::string message = string_format_args(format, args);
    va_end(args);

    auto &writer = get_writer();
    std::string now = now_rfc3339();
    return writer.write(
        string_format("[INFO] %s - %s\n", now.c_str(), message.c_str()));
}

Promise error(char const *format, ...) {
    if (!writer) {
        return resolve();
    }

    va_list args;
    va_start(args, format);
    std::string message = string_format_args(format, args);
    va_end(args);

    auto &writer = get_writer();
    std::string now = now_rfc3339();
    return writer.write(
        string_format("[ERROR] %s - %s\n", now.c_str(), message.c_str()));
}

class StreamWriterImpl {
  public:
    StreamWriterImpl(asio::io_context &context, FILE *file, bool close = false);
    StreamWriterImpl(asio::io_context &context, char const *path,
                     char const *mode);

    ~StreamWriterImpl();
    Promise write(char const *message, std::size_t size);

  private:
    FILE *_file;
    asio::writable_pipe _pipe;
    bool _close;
};

StreamWriterImpl::StreamWriterImpl(asio::io_context &context, FILE *file,
                                   bool close)
    : _file(file), _pipe(context, fileno(file)), _close(close) {}

StreamWriterImpl::StreamWriterImpl(asio::io_context &context, char const *path,
                                   char const *mode)
    : StreamWriterImpl(context, std::fopen(path, mode), true) {}

StreamWriterImpl::~StreamWriterImpl() {
    if (_close) {
        std::fclose(_file);
    }
}

Promise StreamWriterImpl::write(char const *message, std::size_t size) {
    return async_write(_pipe, asio::buffer(message, size));
}

StreamWriter::StreamWriter(asio::io_context &context, FILE *file)
    : _impl(std::make_unique<StreamWriterImpl>(context, file)) {}

StreamWriter::StreamWriter(asio::io_context &context, char const *path,
                           char const *mode)
    : _impl(std::make_unique<StreamWriterImpl>(context, path, mode)) {}

StreamWriter::~StreamWriter() = default;

Promise StreamWriter::write(char const *message, std::size_t size) {
    return _impl->write(message, size);
}

Promise StreamWriter::write(std::string const &message) {
    return _impl->write(message.c_str(), message.size());
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
