cmake_minimum_required(VERSION 3.10)
project(sBeHappy)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.66.0 REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(src)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()