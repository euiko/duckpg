#include <pgwire/pg_wire.hpp>

namespace pgwire {

Message &Message::Message::put_string(std::string const &v) {
    auto p = v.data();
    std::copy(p, p + v.size(), std::back_inserter(_buffer));
    _buffer.push_back('\0');

    return *this;
}

} // namespace pgwire