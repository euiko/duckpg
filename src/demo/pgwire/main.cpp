#include <pgwire/server.hpp>

#include <asio.hpp>

int main(int argc, char **argv) {
    using namespace asio;

    io_context io_context;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 15432);

    duckpg::Server server(io_context, endpoint);
    server.start();
    io_context.run();
    return 0;
}