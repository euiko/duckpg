cmake_minimum_required(VERSION 2.17)
project(duckpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(third_party/asio.cmake)
add_subdirectory(third_party/endian)

include_directories(include)
add_subdirectory(src/pgwire)
add_subdirectory(src/demo)