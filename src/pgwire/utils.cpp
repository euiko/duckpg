#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <iostream>

#include <pgwire/utils.hpp>

namespace pgwire {

std::string string_format(const char* format, ...) {
    // initialize variadic 2 args for 2 pass formatting
    // 1st one is to determince size
    // 2nd one is the one actually write into the buffer
    va_list args, args_pass2;
    va_start (args, format);
    va_copy(args_pass2, args);

    // 1st pass to determine target size
    int size_i = vsnprintf(nullptr, 0, format, args);
    va_end (args);

    if( size_i <= 0 ) {
        throw std::runtime_error( "Error during formatting." );
    }

    auto size = static_cast<size_t>( size_i );
    std::string result;
    result.resize(size);

    // 2nd pass to actually build the string into buffer
    int s_size = vsnprintf(result.data(), size+1, format, args_pass2); // add 1 for null termination
    va_end (args_pass2);

    return std::move(result);
}

}
