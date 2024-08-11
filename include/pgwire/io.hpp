#pragma once

#include <iostream>
#include <pgwire/types.hpp>

#include <asio.hpp>
#include <promise-cpp/promise.hpp>

namespace pgwire {

using promise::all;
using promise::Defer;
using promise::newPromise;
using promise::Promise;
using promise::reject;
using promise::resolve;

// A reference-counted non-modifiable buffer class.
class shared_buffer {
  public:
    // Construct from a Bytes.
    explicit shared_buffer(Bytes &&bytes);

    // Implement the ConstBufferSequence requirements.
    using value_type = asio::const_buffer;
    using const_iterator = const asio::const_buffer;
    const asio::const_buffer *begin() const;
    const asio::const_buffer *end() const;

  private:
    std::shared_ptr<Bytes> _data;
    asio::const_buffer _buffer;
};

shared_buffer make_shared_buffer(Bytes &&bytes);

template <typename Result>
inline void setPromise(Defer defer, asio::error_code err,
                       const Result &result) {
    if (err) {
        std::cerr << "setPromise failed: " << err.message() << "\n";
        defer.reject(err);
    } else
        defer.resolve(result);
}

template <typename Stream, typename Buffer>
inline Promise async_write(Stream &stream, Buffer const &buffer) {
    return newPromise([&](Defer &defer) {
        // write
        asio::async_write(
            stream, buffer,
            [defer](asio::error_code err, std::size_t bytes_transferred) {
                setPromise(defer, err, bytes_transferred);
            });
    });
}

} // namespace pgwire
