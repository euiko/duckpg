#pragma once

#include <string>

#include <endian/network.hpp>
#include <pgwire/types.hpp>
#include <type_traits>

namespace pgwire {
class Buffer {
  public:
    Buffer() = default;
    Buffer(Bytes &&data);

    inline Bytes const &data() const { return _data; }

    Bytes take_bytes();
    size_t size() const;

    Bytes::const_iterator begin() const;
    Bytes::const_iterator end() const;

    Byte const *buffer() const;
    inline Byte at(size_t n) const { return _data[_pos + n]; };
    void advance(size_t n);

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    T get_numeric();
    std::string get_string();

    Buffer &put_byte(Byte b);
    Buffer &put_bytes(Bytes const &bytes);
    Buffer &put_bytes(Byte const *b, std::size_t size);
    Buffer &put_string(std::string_view const &v, bool append_null_char = true);

    template <typename T>
    std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>,
                     Buffer &>
    put_numeric(T v);

  private:
    Bytes _data;
    size_t _pos = 0;
};

template <typename T, typename> T Buffer::get_numeric() {
    T result = endian::network::get<T>(buffer());
    advance(sizeof(T));
    return result;
};

template <typename T>
std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, Buffer &>
Buffer::put_numeric(T v) {
    T buffer = 0;
    endian::network::put(v, reinterpret_cast<uint8_t *>(&buffer));
    auto pointer = reinterpret_cast<uint8_t *>(&buffer);
    std::copy(pointer, pointer + sizeof(T), std::back_inserter(_data));

    return *this;
}
} // namespace pgwire