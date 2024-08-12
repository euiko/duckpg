#pragma once

namespace pgwire::log {

void warning(const char *message, ...);
void info(const char *message, ...);
void error(const char *message, ...);

} // namespace pgwire::log
