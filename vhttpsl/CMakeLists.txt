add_library(vhttpsl)

find_package(OpenSSL REQUIRED)

target_compile_options(vhttpsl PRIVATE -Wall -Werror -ansi -pedantic -D_POSIX_C_SOURCE=200112L)

target_sources(vhttpsl PUBLIC
        include/vhttpsl.h
        include/vhttpsl/server.h
        include/vhttpsl/app.h
        include/vhttpsl/app/request.h
        include/vhttpsl/app/response.h
        include/vhttpsl/http/methods.h
        include/vhttpsl/http/response.h
        include/vhttpsl/http/request.h
        include/vhttpsl/http/headers.h
        include/vhttpsl/http/request_line.h
        include/vhttpsl/http/status.h
        include/vhttpsl/ssl.h
        include/vhttpsl/listener.h
        )

add_subdirectory(src)

target_include_directories(vhttpsl PUBLIC include)

add_subdirectory(example)
add_subdirectory(bits)
add_subdirectory(test)

target_link_libraries(vhttpsl bits streams OpenSSL::SSL OpenSSL::Crypto)