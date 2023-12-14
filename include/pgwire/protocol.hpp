#pragma once

#include <asio.hpp>
#include <endian/network.hpp>

#include <cinttypes>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <pgwire/buffer.hpp>
#include <pgwire/types.hpp>

namespace pgwire {

struct BackendMessage;
struct FrontendMessage;

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

struct SSLResponse;

void encode(Buffer &b, BackendMessage const &msg);
void encode(Buffer &b, SSLResponse const &ssl_resp);

template <typename T> Bytes encode_bytes(T const &t) {
    Buffer b;
    encode(b, t);
    return b.take_bytes();
}

struct BackendMessage {
    virtual BackendTag tag() const noexcept = 0;
    virtual void encode(Buffer &b) const = 0;
};

struct SSLResponse {
    bool support = false;
};

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

struct FieldDescription {
    std::string name;
    Oid oid;
};

using Fields = std::vector<FieldDescription>;

struct RowDescription : public BackendMessage {
    Fields fields;
    FormatCode format_code;

    RowDescription() = default;
    RowDescription(Fields fields, FormatCode format_code = FormatCode::Text);

    BackendTag tag() const noexcept override;
    void encode(Buffer &b) const override;
};

struct CommandComplete : public BackendMessage {
    std::string command_tag;

    CommandComplete() = default;
    CommandComplete(std::string command_tag);

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

struct Terminate : public FrontendMessage {
    FrontendType type() const noexcept override;
    FrontendTag tag() const noexcept override;
    void decode(Buffer &) override;
};

} // namespace pgwire