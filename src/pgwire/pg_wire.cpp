#include <cstdint>
#include <iterator>
#include <optional>
#include <pgwire/pg_wire.hpp>

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

template <> void encode(Buffer &b, BackendMessage const &msg) {
    Buffer body;
    msg.encode(body);

    b.put_numeric<uint8_t>(uint8_t(msg.tag()));
    b.put_numeric<int32_t>(body.size() + sizeof(int32_t));
    b.put_bytes(body.data());
}

template <> void encode(Buffer &b, SSLResponse const &ssl_resp) {
    if (ssl_resp.support) {
        b.put_numeric<uint8_t>('S');
    } else {
        b.put_numeric<uint8_t>('N');
    }
}

BackendTag AuthenticationOk::tag() const noexcept {
    return BackendTag::Authentication;
}

void AuthenticationOk::encode(Buffer &b) const { b.put_numeric<int32_t>(0); }

ParameterStatus::ParameterStatus(std::string name, std::string value)
    : name(std::move(name)), value(std::move(value)) {}

BackendTag ParameterStatus::tag() const noexcept {
    return BackendTag::ParameterStatus;
}

void ParameterStatus::encode(Buffer &b) const {
    b.put_string(name);
    b.put_string(value);
}

ReadyForQuery::ReadyForQuery(Status status) : status(status) {}

BackendTag ReadyForQuery::tag() const noexcept {
    return BackendTag::ReadyForQuery;
}
void ReadyForQuery::encode(Buffer &b) const {
    switch (status) {
    case Idle:
        b.put_numeric('I');
        break;
    case Block:
        b.put_numeric('T');
        break;
    case Failed:
        b.put_numeric('E');
        break;
    }
}

FrontendType StartupMessage::type() const noexcept {
    return is_ssl_request ? FrontendType::SSLRequest : FrontendType::Startup;
}
FrontendTag StartupMessage::tag() const noexcept { return FrontendTag::None; }

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

FrontendType Query::type() const noexcept { return FrontendType::Query; }
FrontendTag Query::tag() const noexcept { return FrontendTag::Query; }
void Query::decode(Buffer &b) { query = b.get_string(); }
} // namespace pgwire