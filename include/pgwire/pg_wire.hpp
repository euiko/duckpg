#pragma once

#include <asio.hpp>
#include <endian/network.hpp>

#include <cinttypes>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace pgwire {

using Byte = uint8_t;
using Bytes = std::vector<Byte>;
using MessageTag = Byte;
using size_t = std::size_t;

struct BackendMessage;
struct FrontendMessage;

enum class FrontendType {
    Invalid,
    Startup,
    SSLRequest,
    Bind,
    Close,
    CopyFail,
    Describe,
    Execute,
    Flush,
    FunctionCall,
    Parse,
    Query,
    Sync,
    Terminate,
    GSSResponse,         // GSS, SASLInitial, SASL is using same tag
    SASLResponse,        // GSS, SASLInitial, SASL is using same tag
    SASLInitialResponse, // GSS, SASLInitial, SASL is using same tag
};

enum class FrontendTag : MessageTag {
    None, // used for frontend message that has no tag e.g. Startup, SSLRequest
    Bind = 'B',
    Close = 'C',
    CopyFail = 'f',
    Describe = 'D',
    Execute = 'E',
    Flush = 'H',
    FunctionCall = 'F',
    Parse = 'P',
    Query = 'Q',
    Sync = 'S',
    Terminate = 'X',
    GSSSASLResponse = 'p', // GSS, SASLInitial, SASL is using same tag
};

enum class BackendTag : MessageTag {
    Authentication = 'R', // AuthenticationOK, KerberosV5, CleartextPassword,
                          // MD5Password, SCMCredential, GSS, GSSContinue, SSPI,
                          // SASL, SASLContinue, SASLFinal is using same tag
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
    template <typename Buffer> size_t write(Buffer &&buffer);
};

using Socket = asio::ip::tcp::socket;

template <>
template <typename Buffer>
size_t Writer<Socket>::write(Buffer &&buffer) {
    return writer.write_some(std::forward<Buffer>(buffer));
}

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

    template <typename T> size_t write(Writer<T> writer);

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

template <typename T> void encode(Buffer &b, T const &t);
template <typename T> Bytes encode_bytes(T const &t) {
    Buffer b;
    encode<T>(b, t);
    return b.take_bytes();
}

struct BackendMessage {
    virtual BackendTag tag() const noexcept = 0;
    virtual void encode(Buffer &b) const = 0;
};

struct SSLResponse {
    bool support = false;
};

template <> void encode(Buffer &b, BackendMessage const &msg);
template <> void encode(Buffer &b, SSLResponse const &ssl_resp);

struct AuthenticationOk : public BackendMessage {
    BackendTag tag() const noexcept override;
    void encode(Buffer &b) const override;
};

struct ParameterStatus : public BackendMessage {
    std::string name;
    std::string value;

    ParameterStatus() = default;
    ParameterStatus(std::string name, std::string value);

    BackendTag tag() const noexcept override;
    void encode(Buffer &b) const override;
};

struct ReadyForQuery : public BackendMessage {
    enum Status { Idle, Block, Failed };
    Status status = Idle;

    ReadyForQuery() = default;
    ReadyForQuery(Status status);

    BackendTag tag() const noexcept override;
    void encode(Buffer &b) const override;
};

struct FrontendMessage {
    virtual FrontendType type() const noexcept = 0;
    virtual FrontendTag tag() const noexcept = 0;
    virtual void decode(Buffer &) = 0;
};

using FrontendMessagePtr = std::unique_ptr<FrontendMessage>;

struct StartupMessage : public FrontendMessage {
    bool is_ssl_request = false;
    int16_t major_version = 0;
    int16_t minor_version = 0;
    std::string user;
    std::string database;
    std::string options; // deprecated

    FrontendType type() const noexcept override;
    FrontendTag tag() const noexcept override;
    void decode(Buffer &) override;
};

struct Query : public FrontendMessage {
    std::string query;

    FrontendType type() const noexcept override;
    FrontendTag tag() const noexcept override;
    void decode(Buffer &) override;
};

} // namespace pgwire