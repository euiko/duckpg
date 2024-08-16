#include <pgwire/io.hpp>

namespace pgwire {

class StreamWriterImpl {
  public:
    StreamWriterImpl(asio::io_context &context, FILE *file, bool close = false);
    StreamWriterImpl(asio::io_context &context, char const *path,
                     char const *mode);

    ~StreamWriterImpl();
    Promise write(char const *message, std::size_t size);

  private:
    FILE *_file;
    asio::writable_pipe _pipe;
    bool _close;
};

shared_buffer::shared_buffer(Bytes &&bytes)
    : _data(std::make_shared<Bytes>(std::move(bytes))),
      _buffer(asio::buffer(*_data)) {}

const asio::const_buffer *shared_buffer::begin() const { return &_buffer; }

const asio::const_buffer *shared_buffer::end() const { return &_buffer + 1; }

shared_buffer make_shared_buffer(Bytes &&bytes) {
    return shared_buffer(std::move(bytes));
}

StreamWriterImpl::StreamWriterImpl(asio::io_context &context, FILE *file,
                                   bool close)
    : _file(file), _pipe(context, fileno(file)), _close(close) {}

StreamWriterImpl::StreamWriterImpl(asio::io_context &context, char const *path,
                                   char const *mode)
    : StreamWriterImpl(context, std::fopen(path, mode), true) {}

StreamWriterImpl::~StreamWriterImpl() {
    if (_close) {
        std::fclose(_file);
    }
}

Promise StreamWriterImpl::write(char const *message, std::size_t size) {
    return async_write(_pipe, asio::buffer(message, size));
}

StreamWriter::StreamWriter(asio::io_context &context, FILE *file)
    : _impl(std::make_unique<StreamWriterImpl>(context, file)) {}

StreamWriter::StreamWriter(asio::io_context &context, char const *path,
                           char const *mode)
    : _impl(std::make_unique<StreamWriterImpl>(context, path, mode)) {}

StreamWriter::~StreamWriter() = default;

Promise StreamWriter::write(char const *message, std::size_t size) {
    return _impl->write(message, size);
}

Promise StreamWriter::write(std::string const &message) {
    return _impl->write(message.c_str(), message.size());
}

} // namespace pgwire
