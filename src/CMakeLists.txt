target_sources(server
        PRIVATE
        cgi.h
        cgi.c
        exec.h
        exec.c
        server.h
        server.c
        options.h
        main.c
        )

add_subdirectory(conf)
add_subdirectory(serve)
add_subdirectory(server)
add_subdirectory(lib)