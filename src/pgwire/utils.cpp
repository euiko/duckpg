#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <pgwire/utils.hpp>

namespace pgwire {

// definitions
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

    return result;
}

std::string string_escape_space(std::string const &s) {
    std::stringstream ss;

    for (auto ch : s) {
        switch (ch) {
        case '\f':
            ss << R"(\f)";
            break;

        case '\n':
            ss << R"(\n)";
            break;

        case '\r':
            ss << R"(\r)";
            break;

        case '\t':
            ss << R"(\t)";
            break;

        case '\v':
            ss << R"(\v)";
            break;

        default:
            ss << ch;
        }
    }

    return ss.str();
}

std::string duration_string(duration_t const &d) {
    using namespace std::chrono;

    std::stringstream ss;

    int64_t h = static_cast<int64_t>(duration_cast<hours>(d).count());
    int64_t m = static_cast<int64_t>(duration_cast<minutes>(d).count() -
                                     (h * 60)); // substract hours
    int64_t s = static_cast<int64_t>(duration_cast<seconds>(d).count() -
                                     (h * 3600 + m * 60));
    // get microseconds that already substracted with h, m, and s
    int64_t us = static_cast<int64_t>(duration_cast<microseconds>(d).count() -
                                      (h * 3600 + m * 60 + s) * 1000);
    double ms = static_cast<double>(us) / 1000.0; // calculate the fractional ms

    bool show_h = h > 0;
    bool show_m = m > 0 || show_h;
    bool show_s = s > 0 || show_m;
    bool show_ms = !show_h && ms > 0;

    if (show_h) {
        ss << h << "h";
    }

    if (show_m) {
        ss << m << "m";
    }

    if (show_s) {
        ss << s << "s";
    }

    if (show_ms) {
        // round to two decimal places
        double rounded = std::round(ms * 100.0) / 100.0;
        ss << rounded << "ms";
    }

    // fallback when there was no case matched to 0s
    if (!show_s && !show_ms) {
        ss << 0 << "s";
    }

    return ss.str();
}

Timer::Timer() : _start(clock_t::now()) {}

duration_t Timer::elapsed() const { return clock_t::now() - _start; }

Timer timer_start() { return Timer{}; }

} // namespace pgwire
