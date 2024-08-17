#include <pgwire/log.hpp>
#include <pgwire/protocol.hpp>
#include <pgwire/server.hpp>
#include <pgwire/types.hpp>
#include <pgwire/writer.hpp>

#include <asio.hpp>

int main(int argc, char **argv) {
    using namespace asio;

    io_context io_context;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 15432);

    int64_t len = 1000;
    if (argc > 1) {
        len = atoll(argv[1]);
    }

    pgwire::log::initialize(io_context);

    pgwire::Server server(io_context, endpoint, [len](pgwire::Session &sess) {
        return [len](std::string const &query) {
            pgwire::PreparedStatement stmt;
            stmt.fields = pgwire::Fields{
                {"name", pgwire::Oid::Text},
                {"address", pgwire::Oid::Text},
                {"age", pgwire::Oid::Int8},
            };
            stmt.handler = [len](pgwire::Writer &writer,
                                 pgwire::Values const &parameters) {
                for (int i = 1; i <= len; i++) {
                    auto row = writer.add_row();
                    row.write_string("euiko");
                    row.write_string("indonesia");
                    row.write_int8(i);
                }
            };
            return stmt;
        };
    });
    server.start();
    io_context.run();
    return 0;
}
