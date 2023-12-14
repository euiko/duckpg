#pragma once

#include <asio.hpp>
#include <endian/big_endian.hpp>

#include <cinttypes>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace pgwire {

using MessageType = uint8_t;

enum class FrontEndMessage : MessageType {
    Bind = 'B',
    Close = 'C',
    CopyFail = 'f',
    Describe = 'D',
    Execute = 'E',
    Flush = 'H',
    FunctionCall = 'F',
    GSSResponse = 'p',
    Parse = 'P',
    Query = 'Q',
    SASLInitialResponse = 'p',
    SASLResponse = 'p',
    Sync = 'S',
    Terminate = 'X'
};

enum class BackEndMessage : MessageType {
    AuthenticationOk = 'R',
    AuthenticationKerberosV5 = 'R',
    AuthenticationCleartextPassword = 'R',
    AuthenticationMD5Password = 'R',
    AuthenticationSCMCredential = 'R',
    AuthenticationGSS = 'R',
    AuthenticationGSSContinue = 'R',
    AuthenticationSSPI = 'R',
    AuthenticationSASL = 'R',
    AuthenticationSASLContinue = 'R',
    AuthenticationSASLFinal = 'R',
    BackendKeyData = 'K',
    BindComplete = '2',
    CloseComplete = '3',
    CommandComplete = 'C',
    CopyData = 'd',
    CopyDone = 'c',
    CopyInResponse = 'G',
    CopyOutResponse = 'H',
    CopyBothResponse = 'W',
    DataRow = 'D',
    EmptyQueryResponse = 'I',
    ErrorResponse = 'E',
    FunctionCallResponse = 'V',
    NegotiateProtocolVersion = 'v',
    NoData = 'n',
    NoticeResponse = 'N',
    NotificationResponse = 'A',
    ParameterDescription = 't',
    ParameterStatus = 'S',
    ParseComplete = '1',
    PortalSuspended = 's',
    ReadyForQuery = 'Z',
    RowDescription = 'T',

};

template <typename T> struct Writer {
    T &writer;
    template <typename Buffer> std::size_t write(Buffer &&buffer);
};

template <typename T> struct AsyncWriter {
    T &writer;
    template <typename Buffer, typename Callback>
    void write_async(Buffer &&buffer, Callback &&);
};

using Socket = asio::ip::tcp::socket;

template <>
template <typename Buffer>
std::size_t Writer<Socket>::write(Buffer &&buffer) {
    return writer.write_some(std::forward<Buffer>(buffer));
}

template <>
template <typename Buffer, typename Callback>
void AsyncWriter<Socket>::write_async(Buffer &&buffer, Callback &&callback) {
    return writer.write_some(std::forward<Buffer>(buffer),
                             std::forward<Callback>(callback));
}

class Message {
  public:
    Message(FrontEndMessage t);
    Message(BackEndMessage t);

    MessageType type() const;
    uint32_t length() const;
    const std::vector<uint8_t> &buffer() const;

    Message &put_string(std::string const &v);

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    Message &put_numeric(T v);

    template <typename T> std::size_t write(Writer<T> writer);
    template <typename T, typename Callback>
    void write_async(AsyncWriter<T> writer, Callback &&callback);

  private:
    MessageType _type;
    std::vector<uint8_t> _buffer;
};

template <typename T, typename> Message &Message::put_numeric(T v) {
    T buffer;
    endian::big_endian::put(v, reinterpret_cast<uint8_t *>(buffer));
    auto pointer = reinterpret_cast<uint8_t *>(&buffer);
    std::copy(pointer, pointer + sizeof(T), std::back_inserter(_buffer));

    return *this;
}

} // namespace pgwire