#include <cstdio>
#include <pgwire/utils.hpp>

#include <cstdarg>
#include <stdexcept>

namespace pgwire {

std::string string_format(const char* format, ...) {
    va_list args;
    va_start (args, format);

    int size_i = vsnprintf(nullptr, 0, format, args);
    if( size_i <= 0 ) {
        throw std::runtime_error( "Error during formatting." );
    }

    auto size = static_cast<size_t>( size_i );
    std::string result;
    result.reserve(size);

    vsnprintf(result.data(), size, format, args);
    va_end (args);
    // copy ellided
    return result;
}

}
