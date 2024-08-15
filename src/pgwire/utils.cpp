#include <cstdio>
#include <stdexcept>

#include <pgwire/utils.hpp>

namespace pgwire {

std::string string_format(const char *format, ...) {
    std::va_list args;

    va_start(args, format);
    std::string result = string_format_args(format, args);
    va_end(args);

    return result;
}

std::string string_format_args(const char *format, std::va_list args) {
    // initialize additional variadic for 2 pass formatting
    // the original args is used to determince size
    // 2nd one is the one actually write into the buffer
    std::va_list args_pass2;
    va_copy(args_pass2, args);

    // 1st pass to determine target size
    int size_i = vsnprintf(nullptr, 0, format, args);

    if (size_i <= 0) {
        throw std::runtime_error("Error during formatting.");
    }

    auto size = static_cast<size_t>(size_i);
    std::string result;
    result.resize(size);

    // 2nd pass to actually build the string into buffer
    vsnprintf(result.data(), size + 1, format,
              args_pass2); // add 1 for null termination
    va_end(args_pass2);

    return std::move(result);
}

} // namespace pgwire
