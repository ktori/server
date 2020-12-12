target_sources(vhttpsl PRIVATE
        app.c
        app.h
        server.c
        server.h
        socket_context.c
        socket_context.h
        ssl.c
        ssl_backend.c
        ssl_backend.h
        )

add_subdirectory(http)
add_subdirectory(listener)