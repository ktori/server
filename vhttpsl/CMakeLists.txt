add_library(vhttpsl)

target_sources(vhttpsl PUBLIC
        include/vhttpsl.h
        include/vhttpsl/server.h
        include/vhttpsl/app.h
        include/vhttpsl/app/request.h
        include/vhttpsl/app/response.h
        include/vhttpsl/http/methods.h
        )
target_sources(vhttpsl PRIVATE src/vhttpsl.c src/server.c src/server.h src/app.c src/app.h)
target_include_directories(vhttpsl PUBLIC include)

add_subdirectory(example)
add_subdirectory(bits)