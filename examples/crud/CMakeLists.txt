add_executable(crud)

add_subdirectory(src)

target_compile_options(crud PRIVATE -Wall -Werror -ansi -pedantic -D_POSIX_C_SOURCE=200112L)

target_link_libraries(crud vhttpsl yaml)