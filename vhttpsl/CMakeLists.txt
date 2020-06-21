add_library(vhttpsl)

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
        )
target_sources(vhttpsl PRIVATE
        src/vhttpsl.c
        src/server.c
        src/server.h
        src/app.c
        src/app.h
        src/http/url.c
        src/socket_context.h
        src/socket_context.c
        src/http/request.c
        src/http/headers.c
        src/http/request_line.c
        src/http/response.c
        src/http/status.c
        src/http/session.c
        src/http/session.h
        src/http/methods.c
        )
target_include_directories(vhttpsl PUBLIC include)

add_subdirectory(example)
add_subdirectory(bits)
add_subdirectory(test)

target_link_libraries(vhttpsl bits)