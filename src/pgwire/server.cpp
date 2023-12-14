#include "asio/buffer.hpp"
#include "asio/completion_condition.hpp"
#include "asio/read.hpp"
#include "asio/streambuf.hpp"
#include "asio/write.hpp"
#include "endian/network.hpp"
#include "pgwire/pg_wire.hpp"
#include <cstddef>
#include <cstdint>
#include <pgwire/server.hpp>

#include <iostream>

#include <asio.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <memory>

using namespace asio;

namespace pgwire {
Session::Session(ip::tcp::socket &&socket)
    : _socket{std::move(socket)}, _startup_done(false), _running(false){

                                                        };
Session::~Session() = default;

void Session::start() {
    _running = true;
    std::cout << "running = " << _running << std::endl;
    for (; _running;) {
        auto msg = this->read();

        assert(msg != nullptr);

        switch (msg->type()) {
        case FrontendType::Invalid:
        case FrontendType::Startup:
            std::cout << "received startup" << std::endl;
            this->write(BackendMessageEncoder<AuthenticationOk>());
            break;
        case FrontendType::SSLRequest:
            std::cout << "received SSL Request" << std::endl;
            this->write(SSLResponse());
            break;
        case FrontendType::Bind:
        case FrontendType::Close:
        case FrontendType::CopyFail:
        case FrontendType::Describe:
        case FrontendType::Execute:
        case FrontendType::Flush:
        case FrontendType::FunctionCall:
        case FrontendType::Parse:
        case FrontendType::Query:
        case FrontendType::Sync:
        case FrontendType::Terminate:
        case FrontendType::GSSResponse:
        case FrontendType::SASLResponse:
        case FrontendType::SASLInitialResponse:
            break;
        }
    }
}

FrontendMessagePtr Session::read() {
    std::cout << "reading startup=" << _startup_done << std::endl;
    if (!_startup_done) {
        return read_startup();
    }

    return nullptr;
}
FrontendMessagePtr Session::read_startup() {
    int32_t len = 0;
    int32_t lenBuf = 0;
    Bytes bytes = {};

    asio::read(_socket,
               buffer(reinterpret_cast<uint8_t *>(&lenBuf), sizeof(int32_t)),
               asio::transfer_exactly(sizeof(int32_t)));
    len = endian::network::get<int32_t>(reinterpret_cast<uint8_t *>(&lenBuf));

    std::size_t size = len - sizeof(int32_t);
    bytes.resize(size);
    asio::read(_socket, buffer(bytes), asio::transfer_exactly(size));

    Buffer buf{std::move(bytes)};
    auto msg = std::make_unique<StartupMessage>();
    msg->decode(buf);

    if (!msg->is_ssl_request)
        _startup_done = true;

    return msg;
}
void Session::write(Encoder const &encoder) {
    Buffer b = encoder.encode();
    asio::write(_socket, buffer(b.data()));
}

Server::Server(io_context &io_context, ip::tcp::endpoint endpoint)
    : _io_context{io_context}, _acceptor{io_context, endpoint} {};

void Server::start() {
    for (;;) {
        accept();
    }
}

void Server::accept() {
    ip::tcp::socket socket(_io_context);
    _acceptor.accept(socket);

    std::thread([s = std::move(socket)]() mutable {
        std::cout << "is open = " << s.is_open() << std::endl;

        Session session(std::move(s));
        session.start();
    }).detach();
}

} // namespace pgwire