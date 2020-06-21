add_executable(vhttpsl_example)

target_compile_options(vhttpsl_example PRIVATE -Wall -Werror -ansi -pedantic -D_POSIX_C_SOURCE=200112L)

target_link_libraries(vhttpsl_example vhttpsl)
target_sources(vhttpsl_example PRIVATE example.c)