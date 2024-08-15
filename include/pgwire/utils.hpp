#pragma once

#include <cstdarg>
#include <string>

namespace pgwire {

std::string string_format(const char *format, ...);

std::string string_format_args(const char *format, std::va_list args);

}; // namespace pgwire
