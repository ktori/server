add_executable(manul)

add_subdirectory(src)

target_compile_options(manul PRIVATE -Wall -Werror -ansi -pedantic -D_POSIX_C_SOURCE=200112L)

target_link_libraries(manul vhttpsl)