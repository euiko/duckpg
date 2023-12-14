#pragma once

#include <pgwire/buffer.hpp>
#include <pgwire/protocol.hpp>
#include <pgwire/types.hpp>

#include <cstdint>
#include <type_traits>

namespace pgwire {
class Writer;
class RowWriter;

void encode(Buffer &b, Writer const &writer);

class Writer {
  public:
    Writer(std::size_t num_cols, FormatCode format_code = FormatCode::Text);

    RowWriter add_row();
    std::size_t num_rows() const;

  private:
    friend void encode(Buffer &b, Writer const &writer);
    friend class RowWriter;

    FormatCode _format_code = FormatCode::Text;
    std::size_t _num_cols = 0;
    std::size_t _num_rows = 0;
    Buffer _data;
};

class RowWriter {
  public:
    RowWriter(Writer &writer);
    ~RowWriter();

    void write_null();
    void write_value(Byte const *b, std::size_t size);
    void write_string(std::string const &value);
    void write_int2(int16_t v);
    void write_int4(int32_t v);
    void write_int8(int64_t v);
    void write_float4(float v);
    void write_float8(double v);

  private:
    Writer &_writer;
    Buffer _row;
    std::size_t _current_col = 0;
};

} // namespace pgwire