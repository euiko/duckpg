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
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    T get_numeric();

    size_t size() const;
    Byte const *buffer() const;
    inline Byte at(size_t n) const { return _data[_pos + n]; };
    void advance(size_t n);

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

struct Encoder {
    virtual ~Encoder() = default;
    virtual Buffer encode() const = 0;
};

struct BackendMessage {
    virtual BackendTag tag() const noexcept = 0;
    virtual Buffer encode() const = 0;
};

template <typename T> struct BackendMessageEncoder : public Encoder {
    static_assert(std::is_base_of<BackendMessage, T>::value);
    template <typename... Args> BackendMessageEncoder(Args &&...args);

    ~BackendMessageEncoder() override = default;
    Buffer encode() const override;

    T _msg;
};

template <typename T>
template <typename... Args>
BackendMessageEncoder<T>::BackendMessageEncoder(Args &&...args)
    : _msg(std::forward<Args>(args)...) {}

template <typename T> Buffer BackendMessageEncoder<T>::encode() const {
    Buffer b;
    Buffer body = _msg.encode();

    b.put_numeric<uint8_t>(uint8_t(_msg.tag()));
    b.put_numeric<int32_t>(body.size() + sizeof(int32_t));
    b.put_bytes(body.data());

    return b;
}

struct SSLResponse : public Encoder {
    bool support = false;

    Buffer encode() const override;
};

struct AuthenticationOk : public BackendMessage {
    inline BackendTag tag() const noexcept override {
        return BackendTag::Authentication;
    };
    Buffer encode() const override;
};

struct FrontendMessage {
    virtual FrontendType type() const noexcept = 0;
    virtual FrontendTag tag() const noexcept = 0;
    virtual void decode(Buffer &) = 0;
};

using FrontendMessagePtr = std::unique_ptr<FrontendMessage>;

struct StartupMessage : FrontendMessage {
    bool is_ssl_request = false;
    int16_t major_version = 0;
    int16_t minor_version = 0;
    std::string user;
    std::string database;
    std::string options; // deprecated

    inline FrontendType type() const noexcept override {
        return is_ssl_request ? FrontendType::SSLRequest
                              : FrontendType::Startup;
    }
    inline FrontendTag tag() const noexcept override {
        return FrontendTag::None;
    };

    void decode(Buffer &) override;
};

} // namespace pgwire