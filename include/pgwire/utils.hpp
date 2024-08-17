#pragma once

#include <chrono>
#include <cstdarg>
#include <string>

namespace pgwire {

using duration_t = std::chrono::nanoseconds;
using clock_t = std::chrono::steady_clock;
using timepoint_t = std::chrono::time_point<clock_t, duration_t>;

class Timer {
  public:
    Timer();
    duration_t elapsed() const;

  private:
    timepoint_t _start;
};

std::string string_format(const char *format, ...);

std::string string_format_args(const char *format, std::va_list args);

std::string string_escape_space(std::string const &s);

std::string duration_string(duration_t const& d);

Timer timer_start();

}; // namespace pgwire
