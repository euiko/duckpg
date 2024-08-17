#pragma once

#include <pgwire/promise.hpp>
#include <asio/io_context.hpp>

// forward declaration
namespace pgwire::io {
    struct Writer;
}

namespace pgwire::log {

Promise warning(char const *format, ...);
Promise info(char const *format, ...);
Promise error(char const *format, ...);

void initialize(asio::io_context &context, const char *file = nullptr);
io::Writer &get_writer();

} // namespace pgwire::log
