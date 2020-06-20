add_library(vhttpsl)

target_sources(vhttpsl PUBLIC include/vhttpsl.h include/vhttpsl/server.h)
target_sources(vhttpsl PRIVATE src/vhttpsl.c src/server.c src/server.h)
target_include_directories(vhttpsl PUBLIC include)

add_subdirectory(example)
add_subdirectory(bits)