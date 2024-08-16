#pragma once

#include <iostream>
#include <memory>

#include <pgwire/promise.hpp>
#include <pgwire/types.hpp>

#include <asio.hpp>

namespace pgwire::io {

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

template <typename Stream, typename Buffer>
inline Promise async_read_exact(Stream &stream, const Buffer &buffer) {
    return newPromise([&](Defer &defer) {
        // read
        asio::async_read(
            stream, buffer, asio::transfer_exactly(buffer.size()),
            [defer](asio::error_code err, std::size_t bytes_transferred) {
                setPromise(defer, err, bytes_transferred);
            });
    });
}

struct Writer {
    virtual ~Writer() = default;
    virtual Promise write(char const *message, std::size_t size) = 0;
    virtual Promise write(std::string const& message) = 0;
};

class StreamWriterImpl;
class StreamWriter : public Writer {
  public:
    StreamWriter(asio::io_context &context, FILE *file = stderr);
    explicit StreamWriter(asio::io_context &context, char const *path,
                          char const *mode);
    ~StreamWriter();

    Promise write(char const *message, std::size_t size) override;
    Promise write(std::string const& message) override;

  private:
    std::unique_ptr<StreamWriterImpl> _impl;
};

} // namespace pgwire
