add_library(bits)

target_compile_options(bits PRIVATE -Wall -Werror -ansi -pedantic -D_POSIX_C_SOURCE=200112L)

target_sources(bits PUBLIC
        include/vhttpsl/bits/shutdown.h
        include/vhttpsl/bits/bytebuf.h
        include/vhttpsl/bits/str.h
        include/vhttpsl/bits/kv.h
        include/vhttpsl/bits/path.h
        include/vhttpsl/bits/minmax.h
        include/vhttpsl/bits/def.h
        )

target_sources(bits PRIVATE
        src/shutdown.c
        src/bytebuf.c
        src/str.c
        src/kv.c
        src/path.c
        )

target_include_directories(bits PUBLIC include)