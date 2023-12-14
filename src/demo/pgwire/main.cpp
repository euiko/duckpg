#include "pgwire/protocol.hpp"
#include "pgwire/types.hpp"
#include "pgwire/writer.hpp"
#include <pgwire/server.hpp>

#include <asio.hpp>

pgwire::PreparedStatement handler(std::string const &query) {
    pgwire::PreparedStatement stmt;
    stmt.fields = pgwire::Fields{
        {"name", pgwire::Oid::Text},
        {"address", pgwire::Oid::Text},
        {"age", pgwire::Oid::Int8},
    };
    stmt.handler = [](pgwire::Writer &writer,
                      pgwire::Values const &parameters) {
        for (int i = 0; i < 10; i++) {
            auto row = writer.add_row();
            row.write_string("euiko");
            row.write_string("indonesia");
            row.write_int8(8 + i);
        }
    };
    return stmt;
};

int main(int argc, char **argv) {
    using namespace asio;

    io_context io_context;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 15432);

    pgwire::Server server(io_context, endpoint,
                          [](pgwire::Session &sess) { return handler; });
    server.start();
    io_context.run();
    return 0;
}