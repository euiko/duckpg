#include <pgwire/buffer.hpp>

namespace pgwire {

Buffer::Buffer(Bytes &&data) : _data(std::move(data)) {}

Bytes Buffer::take_bytes() {
    _pos = 0;
    return std::move(_data);
}

const Byte *Buffer::buffer() const {
    if (_pos >= _data.size()) {
        return nullptr;
    }

    return _data.data() + _pos;
}

void Buffer::advance(size_t n) { _pos += n; }

size_t Buffer::size() const {
    if (_pos >= _data.size()) {
        return 0;
    }
    return _data.size() - _pos;
}

std::string Buffer::get_string() {
    std::string str(reinterpret_cast<const char *>(buffer()));
    advance(str.size() + 1);
    return str;
}

Buffer &Buffer::put_bytes(Bytes const &bytes) {
    _data.reserve(bytes.size() + _data.size());
    std::copy(bytes.begin(), bytes.end(), std::back_inserter(_data));

    return *this;
}
Buffer &Buffer::Buffer::put_string(std::string const &v) {
    auto p = v.data();
    std::copy(p, p + v.size(), std::back_inserter(_data));
    _data.push_back('\0');

    return *this;
}
} // namespace pgwire