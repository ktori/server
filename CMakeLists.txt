cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 90)
project(server)
add_executable(server)

option(SERVER_USE_SSL "Use SSL" ON)
option(USE_FORKS "Use forks" ON)

configure_file(src/options.h.in conf/config.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)

target_compile_options(server PRIVATE -Wall -Werror -ansi -pedantic -D_POSIX_C_SOURCE=200112L)

if (SERVER_USE_SSL)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(server OpenSSL::SSL OpenSSL::Crypto)
endif ()

add_subdirectory(lib/yaml)

add_subdirectory(vhttpsl)

target_link_libraries(server yaml vhttpsl bits)
target_include_directories(server PRIVATE lib)