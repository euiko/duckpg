#include <algorithm>
#include <pgwire/buffer.hpp>

namespace pgwire {

Buffer::Buffer(Bytes &&data) : _data(std::move(data)) {}

Bytes Buffer::take_bytes() {
    _pos = 0;
    return std::move(_data);
}

Bytes::const_iterator Buffer::begin() const {
    if (_pos >= _data.size()) {
        return _data.end();
    }

    return _data.begin() + _pos;
}
Bytes::const_iterator Buffer::end() const { return _data.end(); }

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
    auto it = std::find_if(begin(), end(), [](Byte b) { return b == '\0'; });
    if (it == end()) {
        advance(_data.size() - _pos);
        return "";
    }

    std::string str(begin(), it);
    advance(str.size() + 1);
    return str;
}

Buffer &Buffer::put_byte(Byte b) {
    _data.push_back(b);
    return *this;
}

Buffer &Buffer::put_bytes(Bytes const &bytes) {
    return put_bytes(bytes.data(), bytes.size());
}

Buffer &Buffer::put_bytes(Byte const *b, std::size_t size) {
    _data.reserve(size + _data.size());
    std::copy(b, b + size, std::back_inserter(_data));
    // advance(_data.size());
    return *this;
}

Buffer &Buffer::Buffer::put_string(std::string_view const &v,
                                   bool append_null_char) {
    auto p = v.data();
    _data.reserve(v.size() + _data.size());
    std::copy(p, p + v.size(), std::back_inserter(_data));
    // advance(v.size());

    if (append_null_char) {
        _data.push_back('\0');
        // advance(1);
    }

    return *this;
}
} // namespace pgwire