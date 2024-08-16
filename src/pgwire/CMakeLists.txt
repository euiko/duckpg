add_library(pgwire STATIC
  buffer.cpp
  exception.cpp
  io.cpp
  log.cpp
  protocol.cpp
  server.cpp
  session.cpp
  types.cpp
  utils.cpp
  writer.cpp
)
target_link_libraries(pgwire PUBLIC asio endian function2 promise)

add_library(duckpg::pgwire ALIAS pgwire)
