#pragma once

#include <cstdint>
#include <vector>
namespace pgwire {

using Byte = uint8_t;
using Bytes = std::vector<Byte>;
using MessageTag = Byte;

using size_t = std::size_t;
using oid_t = int32_t;

enum class FrontendType {
    Invalid,
    Startup,
    SSLRequest,
    Bind,
    Close,
    CopyFail,
    Describe,
    Execute,
    Flush,
    FunctionCall,
    Parse,
    Query,
    Sync,
    Terminate,
    GSSResponse,
    SASLResponse,
    SASLInitialResponse,
};

} // namespace pgwire