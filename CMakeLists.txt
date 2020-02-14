cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0076 NEW)

project(server)
add_executable(server)
add_subdirectory(src)
target_compile_options(server PRIVATE -Wall -Werror --std=c89 -D_POSIX_C_SOURCE=200112L)