add_library(pgwire STATIC buffer.cpp protocol.cpp server.cpp)
target_link_libraries(pgwire PUBLIC asio endian)