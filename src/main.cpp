#include <duckpg/server.h>

#include <boost/asio.hpp>

int main(int argc, char **argv) {
    using namespace boost::asio;

    io_context io_context;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 15432);

    duckpg::Server server(io_context, endpoint);
    server.start();
    io_context.run();
    return 0;
}