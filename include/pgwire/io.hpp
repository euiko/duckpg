#pragma once

#include "asio/buffer.hpp"
#include <memory>

#include <pgwire/log.hpp>
#include <pgwire/promise.hpp>
#include <pgwire/types.hpp>

#include <asio.hpp>

namespace pgwire::io {

constexpr std::size_t max_buffer_size = 1024 * 256; // 500KiB

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
inline void set_promise(Defer defer, asio::error_code err,
                        const Result &result) {
    if (err) {
        log::error("set_promise failed: %s", err.message().c_str());
        defer.reject(err);
    } else
        defer.resolve(result);
}

template <typename Stream, typename Buffer>
inline Promise async_write(Stream &stream, Buffer const &buffer) {
    return newPromise([&](Defer &defer) {
        auto actual_size = buffer_size(buffer);
        // actual write
        asio::async_write(
            stream, buffer,
            [defer, &stream, buffer,
             actual_size](asio::error_code err, std::size_t bytes_transferred) {
                if (bytes_transferred == actual_size) {
                    set_promise(defer, err, bytes_transferred);
                } else {
                    auto current_buffer =
                        asio::buffer(buffer + bytes_transferred,
                                     actual_size - bytes_transferred);
                    async_write(stream, current_buffer)
                        .then([defer, bytes_transferred](std::size_t len) {
                            defer.resolve(bytes_transferred + len);
                        })
                        .fail([defer](asio::error_code err) {
                            defer.reject(err);
                        });
                }
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
                set_promise(defer, err, bytes_transferred);
            });
    });
}

struct Writer {
    virtual ~Writer() = default;
    virtual Promise write(char const *message, std::size_t size) = 0;
    virtual Promise write(std::string const &message) = 0;
};

class StreamWriterImpl;
class StreamWriter : public Writer {
  public:
    StreamWriter(asio::io_context &context, FILE *file = stderr);
    explicit StreamWriter(asio::io_context &context, char const *path,
                          char const *mode);
    ~StreamWriter();

    Promise write(char const *message, std::size_t size) override;
    Promise write(std::string const &message) override;

  private:
    std::unique_ptr<StreamWriterImpl> _impl;
};

} // namespace pgwire::io
