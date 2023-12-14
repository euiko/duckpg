add_library(pgwire STATIC pg_wire.cpp server.cpp)
target_link_libraries(pgwire PUBLIC asio endian)