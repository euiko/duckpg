#include <cstdint>
#include <iterator>
#include <optional>
#include <pgwire/pg_wire.hpp>

namespace pgwire {

Buffer::Buffer(Bytes &&data) : _data(std::move(data)) {}

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

Buffer SSLResponse::encode() const {
    Buffer b;
    if (support) {
        b.put_numeric<uint8_t>('S');
    } else {
        b.put_numeric<uint8_t>('N');
    }

    return b;
}

Buffer AuthenticationOk::encode() const {
    Buffer b;
    b.put_numeric<int32_t>(0);
    return b;
}

void StartupMessage::decode(Buffer &b) {
    this->major_version = b.get_numeric<int16_t>();
    this->minor_version = b.get_numeric<int16_t>();

    if (major_version == 1234 && minor_version == 5679) {
        is_ssl_request = true;
        return;
    }

    size_t start = 0;
    std::optional<std::string> key;
    for (size_t i = 0; i < b.size(); i++) {
        auto current = b.at(i);
        // look for null termination
        if (current != '\0')
            continue;

        // found null termination read the string
        std::string value{b.buffer() + start, b.buffer() + start + i};
        start = i + 1;

        // non existing key, put to temporary string
        if (!key) {
            key = std::move(value);
            continue;
        }

        if (key) {
            if (key == "user") {
                this->user = std::move(value);
            } else if (key == "database") {
                this->database = std::move(value);
            } else if (key == "options") {
                this->options = std::move(value);
            }
            // reset the key
            key = std::nullopt;
        }
    };
}

} // namespace pgwire