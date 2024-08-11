#include <pgwire/io.hpp>

namespace pgwire {

shared_buffer::shared_buffer(Bytes &&bytes)
    : _data(std::make_shared<Bytes>(std::move(bytes))),
      _buffer(asio::buffer(*_data)) {}

const asio::const_buffer *shared_buffer::begin() const { return &_buffer; }

const asio::const_buffer *shared_buffer::end() const { return &_buffer + 1; }

shared_buffer make_shared_buffer(Bytes &&bytes) {
    return shared_buffer(std::move(bytes));
}

} // namespace pgwire
