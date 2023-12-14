#pragma once

#include <string>

#include <endian/network.hpp>
#include <pgwire/types.hpp>

namespace pgwire {
class Buffer {
  public:
    Buffer() = default;
    Buffer(Bytes &&data);

    inline Bytes const &data() const { return _data; }

    Bytes take_bytes();
    size_t size() const;
    Byte const *buffer() const;
    inline Byte at(size_t n) const { return _data[_pos + n]; };
    void advance(size_t n);

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    T get_numeric();
    std::string get_string();

    Buffer &put_bytes(Bytes const &bytes);
    Buffer &put_string(std::string const &v);

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    Buffer &put_numeric(T v);

  private:
    size_t _pos = 0;
    Bytes _data;
};

template <typename T, typename> T Buffer::get_numeric() {
    T result = endian::network::get<T>(buffer());
    advance(sizeof(T));
    return result;
};

template <typename T, typename> Buffer &Buffer::put_numeric(T v) {
    T buffer = 0;
    endian::network::put(v, reinterpret_cast<uint8_t *>(&buffer));
    auto pointer = reinterpret_cast<uint8_t *>(&buffer);
    std::copy(pointer, pointer + sizeof(T), std::back_inserter(_data));

    return *this;
}
} // namespace pgwire