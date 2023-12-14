#include <pgwire/protocol.hpp>
#include <pgwire/writer.hpp>

namespace pgwire {

template <typename T>
std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>
write_numeric(Buffer &buffer, FormatCode format_code, T value,
              const char *format) {
    switch (format_code) {
    case FormatCode::Binary:
        buffer.put_numeric<int32_t>(sizeof(T));
        buffer.put_numeric<T>(value);
        break;
    case FormatCode::Text: {
        char buf[256];
        auto len = sprintf(buf, format, value);
        buffer.put_numeric<int32_t>(len);
        buffer.put_bytes(reinterpret_cast<Byte const *>(buf), len);
        break;
    }
    }
}

void encode(Buffer &b, Writer const &writer) {
    b.put_bytes(writer._data.data());
}

Writer::Writer(std::size_t num_cols, FormatCode format_code)
    : _format_code(format_code), _num_cols(num_cols) {}

RowWriter Writer::add_row() {
    RowWriter rowWriter{*this};
    _num_rows++;
    return rowWriter;
}

std::size_t Writer::num_rows() const { return _num_rows; }

RowWriter::RowWriter(Writer &writer) : _writer(writer) {}
RowWriter::~RowWriter() {
    _writer._data.put_numeric<int8_t>(int8_t(BackendTag::DataRow));
    _writer._data.put_numeric<int32_t>(_row.size() + 6);
    _writer._data.put_numeric<int16_t>(_current_col);
    _writer._data.put_bytes(_row.data());
}

void RowWriter::write_value(Byte const *b, std::size_t size) {
    _current_col++;
    _row.put_numeric<int32_t>(size);
    _row.put_bytes(b, size);
}

void RowWriter::write_null() {
    _current_col++;
    _row.put_numeric<int32_t>(-1);
}

void RowWriter::write_string(std::string const &value) {
    write_value(reinterpret_cast<Byte const *>(value.data()), value.size());
}

void RowWriter::write_int2(int16_t v) {
    _current_col++;
    write_numeric(_row, _writer._format_code, v, "%d");
}
void RowWriter::write_int4(int32_t v) {
    _current_col++;
    write_numeric(_row, _writer._format_code, v, "%d");
}
void RowWriter::write_int8(int64_t v) {
    _current_col++;
    write_numeric(_row, _writer._format_code, v, "%lld");
}
void RowWriter::write_float4(float v) {
    _current_col++;
    write_numeric(_row, _writer._format_code, v, "%f");
}
void RowWriter::write_float8(double v) {
    _current_col++;
    write_numeric(_row, _writer._format_code, v, "%f");
}

} // namespace pgwire