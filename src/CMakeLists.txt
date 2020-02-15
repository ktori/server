target_sources(server
        PRIVATE
        cgi.h
        cgi.c
        exec.h
        exec.c
        router.h
        router.c
        server.h
        server.c
        options.h
        main.c
        )

add_subdirectory(serve)
add_subdirectory(server)
add_subdirectory(cluster)
add_subdirectory(lib)