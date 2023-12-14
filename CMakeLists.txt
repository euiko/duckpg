cmake_minimum_required(VERSION 2.17)
project(duckpg LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost)

add_executable(duckpg
    src/main.cpp
    src/server.cpp
)

target_include_directories(duckpg PRIVATE
    include
    ${Boost_INCLUDE_DIRS}
)