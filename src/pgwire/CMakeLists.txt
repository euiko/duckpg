add_library(pgwire STATIC 
  buffer.cpp
  protocol.cpp
  server.cpp
  types.cpp
  writer.cpp
)
target_link_libraries(pgwire PUBLIC asio endian function2)

add_library(duckpg::pgwire ALIAS pgwire)