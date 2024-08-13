#include "pgwire/types.hpp"
#include <cstdint>
#include <optional>
#include <pgwire/protocol.hpp>

namespace pgwire {

void encode(Buffer &b, BackendMessage const &msg) {
    Buffer body;
    msg.encode(body);

    b.put_numeric<uint8_t>(uint8_t(msg.tag()));
    b.put_numeric<int32_t>(body.size() + sizeof(int32_t));
    b.put_bytes(body.data());
}

void encode(Buffer &b, SSLResponse const &ssl_resp) {
    if (ssl_resp.support) {
        b.put_numeric<uint8_t>('S');
    } else {
        b.put_numeric<uint8_t>('N');
    }
}

BackendTag AuthenticationOk::tag() const noexcept {
    return BackendTag::Authentication;
}

void AuthenticationOk::encode(Buffer &b) const { b.put_numeric<int32_t>(0); }

ParameterStatus::ParameterStatus(std::string name, std::string value)
    : name(std::move(name)), value(std::move(value)) {}

BackendTag ParameterStatus::tag() const noexcept {
    return BackendTag::ParameterStatus;
}

void ParameterStatus::encode(Buffer &b) const {
    b.put_string(name);
    b.put_string(value);
}

ReadyForQuery::ReadyForQuery(Status status) : status(status) {}

BackendTag ReadyForQuery::tag() const noexcept {
    return BackendTag::ReadyForQuery;
}
void ReadyForQuery::encode(Buffer &b) const {
    switch (status) {
    case Idle:
        b.put_numeric('I');
        break;
    case Block:
        b.put_numeric('T');
        break;
    case Failed:
        b.put_numeric('E');
        break;
    }
}

RowDescription::RowDescription(std::vector<FieldDescription> fields,
                               FormatCode format_code)
    : fields(std::move(fields)), format_code(format_code) {}

BackendTag RowDescription::tag() const noexcept {
    return BackendTag::RowDescription;
}

void RowDescription::encode(Buffer &b) const {
    b.put_numeric<int16_t>(fields.size());
    for (const auto &field : fields) {
        b.put_string(field.name);
        b.put_numeric<int32_t>(0);
        b.put_numeric<int16_t>(0);
        b.put_numeric(int32_t(field.oid));
        b.put_numeric(get_oid_size(field.oid));
        b.put_numeric<int32_t>(-1);
        b.put_numeric(int16_t(format_code));
    }
}

CommandComplete::CommandComplete(std::string command_tag)
    : command_tag(std::move(command_tag)) {}

BackendTag CommandComplete::tag() const noexcept {
    return BackendTag::CommandComplete;
}
void CommandComplete::encode(Buffer &b) const { b.put_string(command_tag); }

ErrorResponse::ErrorResponse(std::string message, SqlState state,
                             ErrorSeverity severity)
    : message(std::move(message)), severity(severity), sql_state(state) {}

BackendTag ErrorResponse::tag() const noexcept {
    return BackendTag::ErrorResponse;
}
void ErrorResponse::encode(Buffer &b) const {
    b.put_byte('S').put_string(get_error_severity(severity));
    b.put_byte('C').put_string(get_sqlstate_code(sql_state));
    b.put_byte('M').put_string(message);
    b.put_byte(0);
}

FrontendType StartupMessage::type() const noexcept {
    return is_ssl_request ? FrontendType::SSLRequest : FrontendType::Startup;
}
FrontendTag StartupMessage::tag() const noexcept { return FrontendTag::None; }

void StartupMessage::decode(Buffer &b) {
    this->major_version = b.get_numeric<int16_t>();
    this->minor_version = b.get_numeric<int16_t>();

    if (major_version == 1234 && minor_version == 5679) {
        is_ssl_request = true;
        return;
    }

    size_t start = 0;
    std::optional<std::string> key;
    for (size_t i = 0; i < b.size(); i++) {
        auto current = b.at(i);
        // look for null termination
        if (current != '\0')
            continue;

        // found null termination read the string
        std::string value{b.buffer() + start, b.buffer() + i};
        start = i + 1;

        // non existing key, put to temporary string
        if (!key) {
            key = std::move(value);
            continue;
        }

        if (key) {
            if (key == "user") {
                this->user = std::move(value);
            } else if (key == "database") {
                this->database = std::move(value);
            } else if (key == "options") {
                this->options = std::move(value);
            }
            // reset the key
            key = std::nullopt;
        }
    };
}

FrontendType Query::type() const noexcept { return FrontendType::Query; }
FrontendTag Query::tag() const noexcept { return FrontendTag::Query; }
void Query::decode(Buffer &b) { query = b.get_string(); }

FrontendType Terminate::type() const noexcept {
    return FrontendType::Terminate;
}
FrontendTag Terminate::tag() const noexcept { return FrontendTag::Terminate; }
void Terminate::decode(Buffer &b) {}

} // namespace pgwire
