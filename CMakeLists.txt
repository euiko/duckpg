cmake_minimum_required(VERSION 2.17)
project(duckpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(third_party/asio.cmake)
add_subdirectory(third_party/catch2)
add_subdirectory(third_party/endian)
add_subdirectory(third_party/function2)

include_directories(include)

# main project
add_subdirectory(src/pgwire)
add_subdirectory(src/demo)
add_subdirectory(src/duckpg)

# tests
add_subdirectory(test/cpp/pgwire)


export(TARGETS pgwire asio endian NAMESPACE duckpg:: FILE DuckPGTargets.cmake)
