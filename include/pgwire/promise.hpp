#pragma once

#define PROMISE_MULTITHREAD 0
#define PROMISE_HEADONLY
#include <promise-cpp/promise.hpp>

namespace pgwire {
using promise::all;
using promise::Defer;
using promise::handleUncaughtException;
using promise::newPromise;
using promise::Promise;
using promise::reject;
using promise::resolve;
} // namespace pgwire
