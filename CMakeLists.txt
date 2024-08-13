cmake_minimum_required(VERSION 2.17)
project(duckpg LANGUAGES CXX)

# dependencies
option(PROMISE_BUILD_EXAMPLES "Build examples" OFF)
include(third_party/asio.cmake)
add_subdirectory(third_party/catch2)
add_subdirectory(third_party/endian)
add_subdirectory(third_party/function2)
add_subdirectory(third_party/promise-cpp)

# options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# main project
include_directories(include)

add_subdirectory(src/pgwire)
add_subdirectory(src/demo)
add_subdirectory(src/duckpg)

# tests
add_subdirectory(test/cpp/pgwire)

# export
export(TARGETS pgwire asio endian promise NAMESPACE duckpg:: FILE DuckPGTargets.cmake)
