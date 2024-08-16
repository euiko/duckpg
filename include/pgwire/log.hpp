#pragma once

#include <asio/io_context.hpp>
#include <pgwire/io.hpp>

namespace pgwire::log {

Promise warning(char const *format, ...);
Promise info(char const *format, ...);
Promise error(char const *format, ...);

void initialize(asio::io_context &context, const char *file = nullptr);
Writer &get_writer();

} // namespace pgwire::log
