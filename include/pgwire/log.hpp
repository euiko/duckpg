#pragma once

#include <asio/io_context.hpp>
#include <memory>
#include <pgwire/io.hpp>

namespace pgwire::log {

Promise warning(char const *format, ...);
Promise info(char const *format, ...);
Promise error(char const *format, ...);

struct Writer {
    virtual ~Writer() = default;
    virtual Promise write(char const *message, std::size_t size) = 0;
    virtual Promise write(std::string const& message) = 0;
};

class StreamWriterImpl;
class StreamWriter : public Writer {
  public:
    StreamWriter(asio::io_context &context, FILE *file = stderr);
    explicit StreamWriter(asio::io_context &context, char const *path,
                          char const *mode);
    ~StreamWriter();

    Promise write(char const *message, std::size_t size) override;
    Promise write(std::string const& message) override;

  private:
    std::unique_ptr<StreamWriterImpl> _impl;
};

void initialize(asio::io_context &context, const char *file = nullptr);
Writer &get_writer();

} // namespace pgwire::log
